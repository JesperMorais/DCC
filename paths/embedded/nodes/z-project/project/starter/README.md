# Sensor node on Zephyr (native_sim)

A Zephyr app that samples a (pretend) temperature sensor described in the devicetree, processes the samples in a second thread, logs a periodic report and answers shell commands. It runs as a normal Linux program on the `native_sim` board, so no hardware and no cross-compiler are needed. The milestones (in the app) say what each step must do.

Verified on **Zephyr v4.2.0** with the host gcc (Ubuntu, CMake 4.2, Python 3.14 for building and 3.12 for Twister).

## One-time setup (30–45 minutes, mostly downloading)

This follows Zephyr's Getting Started guide for Ubuntu. Other distros: install the same tools with your package manager. The workspace takes several GB.

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

**3. The workspace.** This pins Zephyr 4.2.0, which the project was written against. Any 4.x release should work.

```sh
west init -m https://github.com/zephyrproject-rtos/zephyr --mr v4.2.0 ~/zephyrproject
cd ~/zephyrproject
west update
west zephyr-export
pip install -r ~/zephyrproject/zephyr/scripts/requirements.txt
```

**4. Every new terminal** needs the venv and two variables. This project lives outside the workspace, and `ZEPHYR_BASE` is how west and CMake find Zephyr from here. `ZEPHYR_TOOLCHAIN_VARIANT=host` says "build with the host gcc"; without it CMake stops with *Could not find a package configuration file provided by "Zephyr-sdk"*. (The Makefile sets the second one for you, but the smoke test below runs west directly.) Put these lines in your shell's startup file if you like:

```sh
source ~/zephyrproject/.venv/bin/activate
export ZEPHYR_BASE=~/zephyrproject/zephyr
export ZEPHYR_TOOLCHAIN_VARIANT=host
```

**5. Smoke test.** If this prints `Hello World! native_sim/native`, you're set up (Ctrl-C to stop it):

```sh
west build -p always -b native_sim -d /tmp/hello $ZEPHYR_BASE/samples/hello_world
west build -d /tmp/hello -t run
```

If the build complains about `gnu/stubs-32.h` or `-m32`, the multilib packages are missing. Either install them (`sudo apt install gcc-multilib g++-multilib`), or use the 64-bit board: `-b native_sim/native/64` here, and `make BOARD=native_sim/native/64 ...` in this project (or `export BOARD=native_sim/native/64` once).

## Build, run, test

```sh
make build        # west build -b native_sim -d build/app .
make run          # build and run (Ctrl-C stops it)
make test-m1      # one milestone's tests (m1 to m5)
make test         # every milestone's tests
make twister      # the same tests through Twister, Zephyr's test runner
make clean        # after switching BOARD, or when CMake gets confused
```

The Makefile only wraps west; the commands it runs are printed, so you can run them yourself. A test run ends with a ztest summary per suite and `PROJECT EXECUTION SUCCESSFUL` (or `FAILED`). When something fails, the line above names the test and says what was expected.

**The shell.** On `native_sim` the shell is on a UART, which Zephyr connects to a pseudo-terminal. The first line it prints is `uart connected to pseudotty: /dev/pts/N`; open that in a second terminal with `screen /dev/pts/N` (or `tio`). Or run the program with the UART on your own terminal: `./build/app/zephyr/zephyr.exe -uart_stdinout`. In that mode every log line shows up twice, once from the console and once through the shell, because both now write to your terminal. `zephyr.exe --help` lists every option, such as `-stop_at=5` to stop after 5 simulated seconds.

## How the tests work

`tests/` is a separate Zephyr app. It builds **every `.c` file in `src/` except `main.c`**, plus the ztest suites in `tests/src/`, so:

- Keep `main.c` to `main()`. Anything your node does must start on its own, without `main` calling it.
- The tests use their own devicetree, `tests/app.overlay`, with sensors whose values they know. Your `app.overlay` is only for the real app.
- They load your `prj.conf` and then `tests/prj.conf` on top. The test one swaps the serial shell for a dummy one the tests can type into.
- From milestone 3 on, the tests only talk to your app through the `node` shell command, so how you split the code is up to you.

## Files

- `CMakeLists.txt`: builds every `.c` in `src/`. New files are picked up on the next build.
- `prj.conf`: Kconfig settings for the app.
- `Kconfig`: the app's own options (milestone 1).
- `app.overlay`: the app's devicetree additions (milestone 2).
- `dts/bindings/vnd,ramp-temp.yaml`: the binding for the pretend sensor (given).
- `src/main.c`: the entry point.
- `tests/`: the test app (given). Read a test when you're unsure what's expected.
