The logger had run for weeks. Then an operator typed `systemctl reload logger` during boot, and the daemon died on the spot: SIGHUP arrived a few milliseconds before the code that would have handled it, and SIGHUP's default action is "terminate". A week later the sensor feeder started dying at 3 a.m. with exit status 141. Neither bug was in the event loop. Both were in the **plumbing**: how a daemon starts, how it takes signals, and how it shares a FIFO with programs that come and go.

### main(): arguments in, an exit code out

A daemon is a program like any other. `argc` counts the words on its command line, and `argv[0]` is its own name:

```
$ ./plumbd sensor.fifo plumbd.sock
argc = 3   argv = { "./plumbd", "sensor.fifo", "plumbd.sock", NULL }
```

What `main` returns becomes the **exit status**, and other programs act on it. The shell shows it with `echo $?`, and systemd's `Restart=on-failure` restarts you on anything but 0. The usual convention: **0** means it worked, **1** means it failed (can't open a file, bad config), **2** means it was called wrong (the usage error `grep` and `ls` use). A status above 128 means "killed by signal status − 128": 141 is 128 + 13, **SIGPIPE**. That's the 3 a.m. bug, and we'll get to it.

Keep `main` thin: check the arguments, call the real work, and turn its result into a number. Then the real work is a function you can test.

### FIFOs: a pipe with a name

`mkfifo sensor.fifo` creates a pipe that lives in the filesystem, so unrelated programs can find it. `ls -l` shows it with a `p`:

```
prw-rw-r-- 1 pi pi 0 Oct  6 10:48 sensor.fifo
srwxrwxr-x 1 pi pi 0 Oct  6 10:48 plumbd.sock
```

(The `s` line is a UNIX socket; more on that below.) Try it with two terminals: `echo 42 > sensor.fifo` **hangs** until another terminal runs `cat sensor.fifo`. A FIFO's `open()` is a rendezvous:

| who opens | blocking `open` | with `O_NONBLOCK` |
|---|---|---|
| reader | waits for a writer | returns at once |
| writer | waits for a reader | fails with **`ENXIO`** if there's no reader |

A daemon opens its read end with `O_NONBLOCK`, so startup never waits for a sensor. Once both ends are open, the data rules are:

- **EOF when the last writer closes.** `read()` returns 0 and epoll reports `EPOLLHUP`, and both stay that way **forever**, even after you've seen them. That's the l-epoll gotcha.
- **No reader left means `EPIPE`.** A writer that writes after the last reader closed gets `SIGPIPE`, which kills it (status 141). If it ignores SIGPIPE, `write` returns -1 with `errno == EPIPE` instead.

So how do you stop the EOF spin without ever leaving writers stranded? The tempting fix is "on EOF, close the read end and reopen it". Look at the gap:

```
daemon:  read() → 0 ... close(old) ......... open(new)
                                ↑  no reader  ↑
writer:              open() → ENXIO, or: connected earlier, write() → EPIPE
```

It's a few microseconds, and a sensor that reconnects in a loop will find it. The fix is to swap the order: **open the new read end first**, register it, then remove and close the old one. There's always a reader. A fresh read end doesn't report EOF until a writer has connected and left again, so it doesn't spin. (Linux also lets you open a FIFO `O_RDWR`, which holds a writer of your own so EOF never comes, but POSIX leaves that undefined.)

### Signals as data: block first, then read them

A signal handler runs between any two instructions of your program, so it may only call async-signal-safe functions: no `printf`, no `malloc`, no locks. A daemon that wants to *do* something on SIGHUP, such as reloading its config, would rather handle it in its loop, where it knows what state everything is in. `signalfd` makes that possible:

```c
sigset_t set;
sigemptyset(&set);
sigaddset(&set, SIGTERM); sigaddset(&set, SIGINT); sigaddset(&set, SIGHUP);
sigprocmask(SIG_BLOCK, &set, NULL);                 /* 1. they now wait instead of acting */
int sfd = signalfd(-1, &set, SFD_NONBLOCK | SFD_CLOEXEC);  /* 2. read them as data */
```

The signalfd becomes readable while one of those signals is pending. Each `read` returns a `struct signalfd_siginfo`, and `ssi_signo` says which one. It's one more fd in the epoll set, next to the FIFO.

Two rules make it work:

- **Block first, first thing in `main`.** Until `sigprocmask` runs, SIGHUP still has its default action, and that was the reload bug from the top. Block before you create threads, too: new threads inherit the mask, and a signal sent to the process goes to *any* thread that doesn't block it, where the default action kills everyone.
- **Standard signals don't queue.** A signal is pending or it isn't. Three SIGHUPs before you read are **one** record. Treat SIGHUP as "the config may have changed", never as a counter. Read records until `EAGAIN` each time.

SIGPIPE is the odd one out. A daemon that writes to sockets or pipes usually ignores it (`signal(SIGPIPE, SIG_IGN)`), so a vanished peer costs one `EPIPE` instead of the whole process.

### A UNIX stream socket: a door for questions

A UNIX socket is TCP's interface with a file path instead of a port: no network, permissions from the filesystem, and very fast. The server side is four calls:

```
socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0)   → listener fd
bind(listener, { AF_UNIX, "plumbd.sock" })        → creates the socket file
listen(listener, 8)                               → readable when a client connects
accept(listener, ...)                             → one new fd per client
```

The path goes into `struct sockaddr_un`, whose `sun_path` holds 108 bytes including the `'\0'`. The listener sits in epoll like everything else: readable means "someone is connecting", and `accept` hands you a new fd for that conversation. Try one by hand with `socat - UNIX-CONNECT:plumbd.sock` or `nc -U plumbd.sock`.

The catch is the file. `bind` fails with `EADDRINUSE` if the path exists, and a daemon that crashed (or got SIGKILL) left its socket file behind. So at startup, `lstat` the path:

- **missing**: fine.
- **`S_ISSOCK`**: a leftover, so `unlink` it and bind.
- **anything else**: someone's file, maybe a config file and a typo in your config. Refuse to start and name the path. Never delete what you didn't create.

Use `lstat`, not `stat`: a symlink at that path should be refused, not followed. A clean shutdown closes the listener and unlinks the path.

### Worked example: the shape of the setup

```
main(argc, argv)
  argc wrong?            → usage on stderr, return 2
  block SIGTERM/INT/HUP, signalfd
  epoll_create1
  FIFO: lstat → mkfifo if missing, refuse if not a FIFO; open O_RDONLY|O_NONBLOCK
  socket: lstat → unlink a stale socket, refuse anything else; socket, bind, listen
  any step failed?       → close what's open, message on stderr, return 1
  loop: signalfd → hup++ or stop | FIFO → read, swap on EOF | listener → accept, reply, close
  stop: drain, close everything, unlink the socket, return 0
```

One exit path, fds that start at -1, and a cleanup that closes whatever is `>= 0`: the ownership rule from the boss, now with a socket file to clean up as well.

### Gotchas

- **Writers get `ENXIO`, not a wait,** when they use `O_NONBLOCK` and nobody reads. A shell's `echo > fifo` uses a blocking open, so it waits instead. Same gap, different symptom.
- **Every `accept` is a new fd.** Close it when the conversation ends, on every path, or each question asked leaks one fd.
- **The socket path is relative to the current directory** when it doesn't start with `/`. systemd starts services in `/`, so a real daemon uses absolute paths (often in `/run`).
- **SIGKILL can't be blocked or caught.** That's why the stale-socket check exists.

### In the wild

- **systemd** sends SIGTERM on `stop` and, with `ExecReload=kill -HUP $MAINPID`, SIGHUP on `reload`. `sd-event`, its event loop, uses signalfd exactly like this.
- **Docker, containerd, MySQL, PostgreSQL and X11** all listen on UNIX sockets (`/run/docker.sock`, `/tmp/.X11-unix/X0`), and all of them have stale-socket code.
- **FIFOs** connect shell scripts to daemons on many embedded boards: a GPS daemon feeding a logger, or `mkfifo` plus `tee` to tap a stream. **Interview question:** *"What does a writer see when the reader of a pipe goes away?"* SIGPIPE, or EPIPE if it's ignored, and a daemon should prefer the error.
