# Sensor node on Zephyr (native_sim)

A Zephyr app that samples a (pretend) temperature sensor described in the devicetree, processes the samples in a second thread, logs a periodic report and answers shell commands. It runs as a normal Linux program on the `native_sim` board, so no hardware and no cross-compiler are needed. The milestones (in the app) say what each step must do.

Verified on **Zephyr v4.2.0** with the host gcc (Ubuntu, CMake 4.2, Python 3.14 for building and 3.12 for Twister).

## One-time setup

The lesson **Workshop: a real Zephyr build** walks through this setup with the output you should see at each step, and then has you break a build on purpose so the errors are familiar before the project. If you've done it, skip to step 4 and the smoke test.

Setup takes a while, mostly downloading. That's normal, and it's a one-off. The full workspace takes several GB.

**Linux only.** `native_sim` builds a Linux program, so on macOS or Windows do all of this inside a Linux VM or WSL2 (Ubuntu), and keep the project in the Linux file system (`~/code/...`, not `/mnt/c/...`).

**1. Host tools.** `native_sim` builds with your host gcc, so you don't need the Zephyr SDK (step 4 tells Zephyr so). The plain `native_sim` board builds a 32-bit program, which is what `gcc-multilib` and `g++-multilib` are for. Without them, use the 64-bit board `native_sim/native/64` everywhere instead (see step 5).

```sh
sudo apt update
sudo apt install --no-install-recommends git cmake ninja-build gperf ccache dfu-util \
  device-tree-compiler wget python3-dev python3-venv python3-tk xz-utils file make gcc \
  gcc-multilib g++-multilib libsdl2-dev libmagic1
```

Check that `cmake --version` says 3.20 or newer.

**2. A Python venv with west.** Building works with any recent Python 3. Twister (`make twister`) in Zephyr 4.2 crashes on Python 3.14, so if your `python3` is 3.14, either make the venv with 3.12 or 3.13 (for example `uv venv -p 3.12 ~/zephyrproject/.venv`) or skip Twister and use `make test`, which runs the same tests.

```sh
python3 -m venv ~/zephyrproject/.venv
source ~/zephyrproject/.venv/bin/activate
pip install west
```

In fish, activate with `source ~/zephyrproject/.venv/bin/activate.fish` instead (csh: `activate.csh`).

**3. The workspace.** This pins Zephyr 4.2.0, which the project was written against. Any 4.x release should work.

```sh
west init -m https://github.com/zephyrproject-rtos/zephyr --mr v4.2.0 ~/zephyrproject
cd ~/zephyrproject
west update
west zephyr-export
pip install -r ~/zephyrproject/zephyr/scripts/requirements.txt
```

*A slimmer workspace.* `west update` fetches every vendor HAL and library, and `native_sim` needs none of them. If disk or bandwidth is tight, fetch only Zephyr itself instead of the block above:

```sh
mkdir -p ~/zephyrproject && cd ~/zephyrproject
git clone --depth 1 --branch v4.2.0 https://github.com/zephyrproject-rtos/zephyr.git
west init -l zephyr
pip install -r zephyr/scripts/requirements.txt
```

That's enough for this project. For a real board later, run `west update` then.

**4. Every new terminal** needs the venv and two variables. This project lives outside the workspace, and `ZEPHYR_BASE` is how west and CMake find Zephyr from here. `ZEPHYR_TOOLCHAIN_VARIANT=host` says "build with the host gcc"; without it CMake stops with *Could not find a package configuration file provided by "Zephyr-sdk"*. (The Makefile sets the second one for you, but the smoke test below runs west directly.) Put these lines in your shell's startup file if you like:

```sh
source ~/zephyrproject/.venv/bin/activate
export ZEPHYR_BASE=~/zephyrproject/zephyr
export ZEPHYR_TOOLCHAIN_VARIANT=host
```

In fish: `source ~/zephyrproject/.venv/bin/activate.fish`, then `set -gx ZEPHYR_BASE ~/zephyrproject/zephyr` and `set -gx ZEPHYR_TOOLCHAIN_VARIANT host`.

**5. Smoke test.** If this prints `Hello World! native_sim/native`, you're set up (Ctrl-C to stop it):

```sh
west build -p always -b native_sim -d /tmp/hello $ZEPHYR_BASE/samples/hello_world
west build -d /tmp/hello -t run
```

If the build complains about `gnu/stubs-32.h` or `-m32`, the multilib packages are missing. Either install them (`sudo apt install gcc-multilib g++-multilib`), or use the 64-bit board: `-b native_sim/native/64` here, and `make BOARD=native_sim/native/64 ...` in this project (or `export BOARD=native_sim/native/64` once).

### Stuck at setup? Check in this order

Each row assumes the ones above it are fine. Stop at the first one that isn't.

| Check | What you should see | If not |
|---|---|---|
| Is the venv active? | your prompt starts with `(.venv)` | `source ~/zephyrproject/.venv/bin/activate` (fish: `activate.fish`). `west: command not found` means this. |
| `west --version` | `West version: v1.x` | `pip install west` inside the venv |
| `echo $ZEPHYR_BASE` | `/home/you/zephyrproject/zephyr` | the `export` from step 4. Outside the workspace, a missing `ZEPHYR_BASE` gives *west: unknown command "build"; do you need to run this inside a workspace?* |
| `cmake --version` | 3.20 or newer | install a newer CMake (`pip install cmake` in the venv works) |
| the `hello_world` smoke test | `Hello World! native_sim/native` | read the **first** error in the build log, not the last. `Zephyr-sdk` means step 4's toolchain variable; `stubs-32.h` means multilib |
| this project | `make build` works | `make clean`, then build again. CMake caches the board and which overlay files exist, so after a switch or a new `app.overlay`, a clean build is the fix |

## Build, run, test

```sh
make build        # west build -b native_sim -d build/app .
make run          # build and run (Ctrl-C stops it)
make test-m1      # one milestone's tests (m1 to m5)
make test-m2 TEST=m2_driver::test_ramp_steps_and_wraps   # one test by name
make test         # every milestone's tests
make twister      # the same tests through Twister, Zephyr's test runner
make clean        # after switching BOARD, or when CMake gets confused
```

The Makefile only wraps west; the commands it runs are printed, so you can run them yourself. A test run ends with a ztest summary per suite and `PROJECT EXECUTION SUCCESSFUL` (or `FAILED`). `TEST=` takes `suite::test`, a comma-separated list, or `suite::*` for a whole suite; the suite and test names are in the summary.

**The shell.** On `native_sim` the shell is on a UART, which Zephyr connects to a pseudo-terminal. The first line it prints is `uart connected to pseudotty: /dev/pts/N`; open that in a second terminal with `screen /dev/pts/N` (or `tio`). Or run the program with the UART on your own terminal: `./build/app/zephyr/zephyr.exe -uart_stdinout`. In that mode every log line shows up twice, once from the console and once through the shell, because both now write to your terminal. `zephyr.exe --help` lists every option, such as `-stop_at=5` to stop after 5 simulated seconds.

## How the tests work

`tests/` is a separate Zephyr app. It builds **every `.c` file in `src/` except `main.c`**, plus the ztest suites in `tests/src/`, so:

- Keep `main.c` to `main()`. Anything your node does must start on its own, without `main` calling it.
- The tests use their own devicetree, `tests/app.overlay`, with sensors whose values they know. Your `app.overlay` is only for the real app.
- They load your `prj.conf` and then `tests/prj.conf` on top. The test one swaps the serial shell for a dummy one the tests can type into, and turns on kernel asserts (`CONFIG_ASSERT=y`), so a mutex taken in a timer expiry stops the run with *ASSERTION FAIL ... mutexes cannot be used inside ISRs*. The app you `make run` has asserts off; add `CONFIG_ASSERT=y` to your `prj.conf` while you develop if you want the same safety net there.
- From milestone 3 on, the tests only talk to your app through the `node` shell command, so how you split the code is up to you.

## APIs you'll need

The lessons cover the kernel objects. These are the pieces they didn't, with where to read more. Read the doc page, then the matching example in the tests or in Zephyr's `samples/`.

| For | Names | Docs |
|---|---|---|
| The driver (m2) | `DT_DRV_COMPAT`, `DT_INST_PROP`, `DEVICE_DT_INST_DEFINE`, `DT_INST_FOREACH_STATUS_OKAY`, `DEVICE_API(sensor, ...)`, `struct sensor_value` | [Implementing fetch and get](https://docs.zephyrproject.org/latest/hardware/peripherals/sensor/index.html#implementing-fetch-and-get), [devices from instance numbers](https://docs.zephyrproject.org/latest/build/dts/howtos.html#option-1-create-devices-using-instance-numbers) |
| Threads and the queue (m3) | `K_THREAD_DEFINE`, `K_MSGQ_DEFINE`, `k_msgq_put`/`k_msgq_get`, `K_MUTEX_DEFINE`, `K_TIMEOUT_ABS_MS` | [Message queues](https://docs.zephyrproject.org/latest/kernel/services/data_passing/message_queues.html), [Mutexes](https://docs.zephyrproject.org/latest/kernel/services/synchronization/mutexes.html) |
| The shell command (m3, m5) | `SHELL_STATIC_SUBCMD_SET_CREATE` (the list of subcommands), `SHELL_CMD` and `SHELL_CMD_ARG` (one subcommand; the `_ARG` form says how many arguments it needs), `SHELL_SUBCMD_SET_END`, `SHELL_CMD_REGISTER` (the root command), `shell_print` and `shell_error` (normal and error output) | [Shell: creating commands](https://docs.zephyrproject.org/latest/services/shell/index.html#creating-commands) |
| Logging (m1, m4) | `LOG_MODULE_REGISTER` once per module (the name shows in each line, like `<inf> reporter:`), `LOG_MODULE_DECLARE` in a second file of the same module, `LOG_INF`/`LOG_ERR` | [Logging in a module](https://docs.zephyrproject.org/latest/services/logging/index.html#logging-in-a-module) |
| The report timer (m4) | `K_TIMER_DEFINE`, `k_timer_start`, `K_WORK_DEFINE`, `k_work_submit`, `SYS_INIT` (run a function at boot without `main`) | [Timers](https://docs.zephyrproject.org/latest/kernel/services/timing/timers.html), [Workqueues](https://docs.zephyrproject.org/latest/kernel/services/threads/workqueue.html), [System drivers and SYS_INIT](https://docs.zephyrproject.org/latest/kernel/drivers/index.html#system-drivers) |
| Parsing `node period` (m5) | `strtol` with its end pointer: it tells you where the number stopped, so `"200x"` is caught | [strtol](https://en.cppreference.com/w/c/string/byte/strtol) |

## When you're stuck

1. **Read the first failing test only.** Scroll up from the summary to the first `FAIL -` line. Just above it is `Assertion failed at .../test_m2.c:21: read_milli: (sensor_sample_fetch(dev) is non-zero)` and a sentence like `sensor_sample_fetch(fresh-sensor) failed`. The file and line say which check failed, the brackets say what was compared, and the sentence says what was expected and, where it can, what it got. Open the test at that line. If the build fails instead, read the **first** `error:` in the log; the rest usually follow from it.
2. **Run just that milestone** (`make test-m2`), or one test by name (`make test-m2 TEST=m2_driver::test_ramp_steps_and_wraps`).
3. **Look at the value.** `printk("index=%d\n", data->index);` prints straight to your terminal, in the app and in the tests. `zephyr.exe` is an ordinary Linux program, so gdb works too: `gdb --args build/tests-m2/zephyr/zephyr.exe -test=m2_driver::test_ramp_steps_and_wraps`, then `break ramp_temp_sample_fetch`, `run`, `next`, `print *data`. If gdb says *value has been optimized out*, add `CONFIG_DEBUG_OPTIMIZATIONS=y` to `prj.conf` while you debug. For a build problem, look at what the build produced: `build/app/zephyr/.config` (the Kconfig values it settled on) and `build/app/zephyr/zephyr.dts` (the merged devicetree).
4. **Make the step smaller.** One function, one test, one line at a time. For the driver: first make `fetch` return 0 for the right channels and see which test moves, then the value, then the wrap.
5. **Take a hint.** Hints are a tool, not a failure. They point at the lesson to reread.
6. **Commit when green.** `git init` once, then `git add -A && git commit -m "m2 green"` after each milestone, so you can always get back to a working version. (`.gitignore` already leaves out `build/`.)
7. **Walk away for ten minutes.** Seriously. Most bugs are found on the way back.

## Files

- `CMakeLists.txt`: builds every `.c` in `src/`. New files are picked up on the next build.
- `prj.conf`: Kconfig settings for the app.
- `Kconfig`: the app's own options (milestone 1).
- `app.overlay`: the app's devicetree additions (milestone 2).
- `dts/bindings/vnd,ramp-temp.yaml`: the binding for the pretend sensor (given).
- `src/main.c`: the entry point.
- `src/ramp_temp.c`: the driver skeleton (milestone 2). The device plumbing is there; the TODOs are the sensor's behaviour.
- `tests/`: the test app (given). Read a test when you're unsure what's expected.
