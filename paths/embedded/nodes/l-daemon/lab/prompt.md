A small daemon on a gateway, `plumbd`, needs the three pieces of plumbing every Linux service ends up with: a **FIFO** that a sensor process writes lines into, **signals** from systemd (stop, reload), and a **UNIX socket** where a technician can ask how it's doing. All three go into one `epoll` loop.

```c
struct plumb { int ep, sig, fifo, listener; char fifo_path[108], sock_path[108]; int lines, hups; };

int  plumb_open(struct plumb *p, const char *fifo_path, const char *sock_path);
int  plumb_run(struct plumb *p);
void plumb_close(struct plumb *p);
int  plumb_main(int argc, char **argv);
```

**`plumb_open`** sets everything up and returns 0, or a negative errno.
- **Signals first.** Block SIGTERM, SIGINT and SIGHUP with `sigprocmask`, then read them through a `signalfd`. After `plumb_open` returns, those signals never kill the process; they wait in the signalfd.
- **The FIFO.** Create it with `mkfifo` if the path doesn't exist. If something that isn't a FIFO is there, return `-EEXIST`. Open the read end without waiting for a writer.
- **The socket.** Listen on a UNIX stream socket at `sock_path`. A socket file already there is a leftover from a crash: replace it. Anything else at that path is someone's file: return `-EEXIST` and leave it alone.
- On failure, close whatever it already opened (the tests count fds) and don't create the socket file.

**`plumb_run`** runs the loop until SIGTERM or SIGINT and returns 0 (or a negative errno if something breaks).
- **FIFO:** count the newline bytes into `lines`. Writers connect and leave whenever they like. A writer that connects must **always** find a reader (a non-blocking `open` for writing fails with `ENXIO` when there's none), and EOF must not make the loop spin.
- **SIGHUP:** `hups++`. **SIGTERM or SIGINT:** stop, but first handle everything that is already waiting, like the l-epoll loop's drain.
- **A client connecting** to the socket gets one line, `lines=<n> hups=<n>\n`, and is then disconnected.
- The struct stays usable: the tests call `plumb_run` several times on one `plumb`.

**`plumb_close`** closes every fd it opened and removes the socket file. The FIFO stays.

**`plumb_main`** is the daemon's `main()`, with the usage `plumbd <fifo> <socket>`. The wrong number of arguments prints a usage line to stderr and returns **2**. If `plumb_open` fails, it prints why to stderr and returns **1**. Otherwise it runs, closes and returns **0**.

The tests are deterministic: they raise the signals with `raise()` (blocked, so they wait) *before* calling `plumb_run`, so each run handles what's queued and returns.

```
writer sends "21.5\n22.0\n22." then "5\n"; raise(SIGTERM)
plumb_run(&p)  → 0, p.lines == 3

raise(SIGHUP); raise(SIGHUP); raise(SIGTERM)
plumb_run(&p)  → 0, p.hups == 1   (a standard signal is pending or not: two become one)

a client connects; raise(SIGTERM); plumb_run(&p)
the client reads "lines=3 hups=1\n", then EOF
```
