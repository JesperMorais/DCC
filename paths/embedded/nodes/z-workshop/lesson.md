### The afternoon before the first line of code

Ask anyone who started on Zephyr what their first day was like. It wasn't threads or devicetree. It was `west: command not found`, then a CMake error about a "Zephyr-sdk" they never installed, then a build that ignored the overlay they'd just written. None of that is hard once you've seen it, so this workshop has you see it on purpose, on your own machine, before the project. Type along. Slow is fine; reading the errors is the point.

### Set up once

`native_sim` builds Zephyr as an ordinary **Linux** program with your host gcc: no board, no SDK. On macOS or Windows, work inside WSL2 or a Linux VM. On Ubuntu:

```sh
sudo apt install --no-install-recommends git cmake ninja-build gperf ccache \
  device-tree-compiler python3-dev python3-venv xz-utils file make gcc gcc-multilib g++-multilib
python3 -m venv ~/zephyrproject/.venv
source ~/zephyrproject/.venv/bin/activate      # fish: activate.fish
pip install west
west init -m https://github.com/zephyrproject-rtos/zephyr --mr v4.2.0 ~/zephyrproject
cd ~/zephyrproject && west update
pip install -r zephyr/scripts/requirements.txt
```

`west update` downloads every vendor HAL, and `native_sim` needs none of them. The slim alternative is just Zephyr: `git clone --depth 1 --branch v4.2.0 https://github.com/zephyrproject-rtos/zephyr.git` inside `~/zephyrproject`, then `west init -l zephyr`. That's how this lesson's outputs were made.

Every new terminal then needs:

```sh
source ~/zephyrproject/.venv/bin/activate
export ZEPHYR_BASE=~/zephyrproject/zephyr     # fish: set -gx ZEPHYR_BASE ...
export ZEPHYR_TOOLCHAIN_VARIANT=host          # build with the host gcc
```

`gcc-multilib` is there because plain `native_sim` is a 32-bit program. Without it, use the board `native_sim/native/64` everywhere.

### Stuck at setup? Check in this order

| Check | Expect | Typical symptom when it's wrong |
|---|---|---|
| venv active | `(.venv)` in the prompt | `west: command not found` |
| `west --version` | `West version: v1.5.0` | not installed in this venv |
| `echo $ZEPHYR_BASE` | `.../zephyrproject/zephyr` | `west: unknown command "build"; do you need to run this inside a workspace?` |
| `cmake --version` | 3.20 or newer | `CMake 3.20.0 or higher is required` |
| hello_world (below) | `Hello World!` | `Could not find a package configuration file provided by "Zephyr-sdk"`: the toolchain variable. `gnu/stubs-32.h`: multilib |
| your app | builds | delete `build/` (or `-p always`) and build again |

Stop at the first row that's wrong. The later rows can't pass until it does.

### Build and run hello_world

```
$ west build -p always -b native_sim -d /tmp/hello $ZEPHYR_BASE/samples/hello_world
-- Zephyr version: 4.2.0 (/home/you/zephyrproject/zephyr)
-- Board: native_sim, qualifiers: native
-- Found toolchain: host (gcc/ld)
...
[88/88] Running utility command for native_runner_executable
$ /tmp/hello/zephyr/zephyr.exe -stop_at=1
*** Booting Zephyr OS build v4.2.0 ***
Hello World! native_sim/native

Stopped at 1.010s
```

`-p always` is a **pristine** build: throw away the old build directory first. `-stop_at=1` stops after one simulated second; without it the kernel idles until Ctrl-C.

### Your own app, your own option

Copy `samples/hello_world` to `~/code/hello`, outside the workspace. Add a `Kconfig` next to `CMakeLists.txt`:

```kconfig
config GREET_COUNT
	int "How many times to greet"
	range 1 10
	default 3

source "Kconfig.zephyr"
```

Loop `CONFIG_GREET_COUNT` times in `main.c`, put `CONFIG_GREET_COUNT=50` in `prj.conf`, and build:

```
warning: user value 50 on the int symbol GREET_COUNT (defined at .../hello/Kconfig:3)
ignored due to being outside the active range ([1, 10]) -- falling back on defaults

error: Aborting due to Kconfig warnings
```

Read it left to right: which symbol, where it's defined, what you asked for, what was allowed. Set it to 2 and the build goes through. Then look at what Kconfig decided in `build/zephyr/.config` (`CONFIG_GREET_COUNT=2`) and what your C sees in `build/zephyr/include/generated/zephyr/autoconf.h` (`#define CONFIG_GREET_COUNT 2`).

### A devicetree error

Add an `app.overlay` with an alarm LED on the emulated GPIO controller, with one letter missing from the compatible:

```dts
/ {
	aliases { alarm-led = &alarm_led; };
	alarm_leds {
		compatible = "gpio-led";          /* should be "gpio-leds" */
		alarm_led: led_alarm { gpios = <&gpio0 5 GPIO_ACTIVE_HIGH>; };
	};
};
```

and use it with `GPIO_DT_SPEC_GET(DT_ALIAS(alarm_led), gpios)` (plus `CONFIG_GPIO=y`). Rebuild. The first surprise: nothing about the overlay happens. The log has no `Found devicetree overlay` line, because CMake decided which overlay files exist when it first set up this build directory, and it remembers that. A **new** overlay file needs a pristine build. With `-p always`:

```
error: 'DT_N_S_alarm_leds_S_led_alarm_P_gpios_IDX_0_VAL_pin' undeclared here (not in a function);
did you mean 'DT_N_S_leds_S_led_0_P_gpios_IDX_0_VAL_pin'?
```

Decode the name: node `/alarm_leds/led_alarm`, property (`P_`) `gpios`, cell `pin`. The node exists but its `gpios` macros don't, and the "did you mean" points at the board's LED, which has them. Now open `build/zephyr/zephyr.dts`, the merged tree, where each line says which file it came from:

```
	alarm_leds {
		compatible = "gpio-led"; /* in ../hello/app.overlay:7 */
```

No binding matches `gpio-led`, so nothing told the generator what `gpios` is. Fix the spelling and it runs: `alarm LED: pin 5 on gpio_emul`.

### Reading ztest output

A test app is the same kind of project with `CONFIG_ZTEST=y` and `ZTEST(suite, name)` functions. Here `average(10, 20)` has a bug:

```
START - test_average_of_two_values

    Assertion failed at .../src/test_math.c:17: workshop_test_average_of_two_values: (got not equal to 15)
average(10, 20) should be 15, got 20
 FAIL - test_average_of_two_values in 0.000 seconds
...
SUITE FAIL -  50.00% [workshop]: pass = 1, fail = 1, skip = 0, total = 2
PROJECT EXECUTION FAILED
```

Line 17 of the test, the comparison that failed, and the message with what came back. Rerun only that test while you fix it: `zephyr.exe -test=workshop::test_average_of_two_values`.

### In the wild

- **Onboarding docs** at Zephyr shops are mostly this page: pinned versions, the environment lines, and a "known errors" table. Many teams ship a container or a `west` manifest so nobody hand-installs anything.
- **CI** builds with `west build -p always` for exactly the stale-cache reason above, and runs ztest suites on `native_sim` through Twister before anything touches hardware.
- **`zephyr.dts` and `.config`** are the first files a senior engineer opens on a "works on my board" bug: they show what the build actually decided.
