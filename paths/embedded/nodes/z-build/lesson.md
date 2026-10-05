### "It builds on my machine" (it didn't)

Every Zephyr developer has lived this one. You add `#include <zephyr/drivers/sensor.h>`, grab the sensor with `DEVICE_DT_GET()`, and get a *linker* error about some `__device_dts_ord_42` you've never heard of. The C is fine. The driver simply isn't in the image, because nobody **asked** for it. In Zephyr the code you write is only one of four inputs to the build. Learn the other three and the mystery errors go away.

In Fundamentals you wrote `rtos.h` code and pressed Run. A real Zephyr product is assembled from roughly 2 million lines of kernel, drivers, protocol stacks and HALs, and the build's job is to throw away everything you *didn't* ask for.

### west: the meta-tool

`west` is Zephyr's command-line front door. It does two jobs:

1. **Workspace manager.** A Zephyr *workspace* is many git repositories: `zephyr` itself, vendor HALs (`hal_nordic`, `hal_stm32`), `mcuboot`, `mbedtls`, and so on. The list lives in a **manifest**, `west.yml`, which pins each project to a revision. `west init` creates the workspace from a manifest and `west update` checks out every project at the pinned revision. Your product repo usually *is* the manifest repo, so a single `west.yml` commit pins the whole firmware's dependency tree.
2. **Command runner.** `west build -b nrf52840dk/nrf52840 app` configures and builds. `west flash` programs the board through the right runner (J-Link, pyOCD, OpenOCD...), and `west debug` starts a GDB session.

```yaml
# west.yml in your product repo
manifest:
  projects:
    - name: zephyr
      url: https://github.com/zephyrproject-rtos/zephyr
      revision: v3.7.0
      import: true        # pull in Zephyr's own module list
  self:
    path: pump-firmware
```

### The build pipeline

`west build` is a wrapper around CMake. Here's what happens, in order:

```
 board + overlays ──▶ devicetree (.dts) ──▶ devicetree_generated.h
                          │                   (DT_* macros)
 Kconfig defaults ─┐      ▼
 board defconfig ──┼──▶ Kconfig resolve ──▶ .config ──▶ autoconf.h
 prj.conf ─────────┘      (CONFIG_* symbols)            (#define CONFIG_*)
                                   │
                                   ▼
            CMake picks sources: zephyr_library_sources_ifdef(CONFIG_FOO ...)
                                   │
                                   ▼
              compile + link ──▶ zephyr.elf ──▶ zephyr.hex / .bin
```

1. **Devicetree** is processed first. It describes the *hardware*: which I2C controller is at which address, which pin the LED is on. The output is a header full of `DT_*` macros.
2. **Kconfig** runs next. It describes the *software configuration*: which subsystems, drivers and features are built in, and their sizes. The output is `.config` plus `autoconf.h` with `#define CONFIG_...` lines. Devicetree feeds into it: many drivers default to `y` only if a matching, enabled node exists in the devicetree (`DT_HAS_..._ENABLED`).
3. **CMake** uses those symbols to decide which `.c` files to compile at all.
4. **Compile and link.**

So the hardware description and the feature selection are both resolved *before a single line of your C is compiled*. That's why Zephyr can be small: unused drivers aren't just dead code, they never enter the build.

### prj.conf, menuconfig, and Kconfig logic

Your app's `prj.conf` is a list of Kconfig assignments:

```ini
CONFIG_GPIO=y
CONFIG_I2C=y
CONFIG_SENSOR=y
CONFIG_BT=y
CONFIG_BT_PERIPHERAL=y
CONFIG_MAIN_STACK_SIZE=2048
CONFIG_LOG=y
```

These are **requests, not orders**. Kconfig symbols have dependencies:

```kconfig
config BME280
    bool "BME280 sensor"
    default y
    depends on DT_HAS_BOSCH_BME280_ENABLED
    depends on SENSOR
    select I2C if $(dt_compat_on_bus,$(DT_COMPAT_BOSCH_BME280),i2c)
```

- **`depends on`** sets a precondition. If `SENSOR` is `n`, then `BME280` *cannot* be `y`, whatever you write. Kconfig drops your request, prints a warning and **carries on building**.
- **`select`** forces another symbol on. Enabling `BME280` on an I2C bus turns `I2C` on for you. (`select` ignores the selected symbol's own dependencies, which is why it's used sparingly.)

Other mistakes in `prj.conf` stop the build instead, with *"error: Aborting due to Kconfig warnings"* (checked on Zephyr 4.2): assigning a symbol that doesn't exist (a typo such as `CONFIG_SENOSR=y`), an `int` outside its `range`, or a symbol with no prompt, such as `DT_HAS_BOSCH_BME280_ENABLED`, which only Kconfig itself sets. The unmet `depends on` is the one that lets the build succeed, which makes it the dangerous one.

`west build -t menuconfig` (or `guiconfig`) opens an interactive editor for the resolved configuration. Press `?` on a symbol and it shows *why* it has its value and what it depends on. Changes there are temporary (they live in `build/zephyr/.config`), so copy what you learn back into `prj.conf`.

### Boards and overlays

A **board** (`nrf52840dk/nrf52840`, `nucleo_f429zi`) brings its own devicetree (`.dts`) and Kconfig defaults (`_defconfig`). You rarely edit those. Instead you layer on top:

- `boards/<board>.overlay`: devicetree additions for your PCB (a sensor on `i2c0`, a button pin), picked up automatically for that board.
- `boards/<board>.conf`: Kconfig additions for that board only.
- `-DEXTRA_CONF_FILE=debug.conf` and `-DEXTRA_DTC_OVERLAY_FILE=...` add build variants, such as a debug image with logging and a release image without.

### Worked example: "why isn't my driver enabled?"

You add a BME280 to your custom board's overlay, write `CONFIG_BME280=y`, and use `DEVICE_DT_GET(DT_NODELABEL(bme280))`. The link fails with `undefined reference to '__device_dts_ord_42'`. That symbol is the device instance a driver would have defined for your node, so no driver did. Debug it in order:

1. `build/zephyr/.config`: is `CONFIG_BME280=y` there? If it isn't, a dependency failed. The build log has a warning like *"BME280 was assigned the value 'y' but got the value 'n'. Check these unsatisfied dependencies: SENSOR (=n)"*, and the build went on without the driver.
2. The warning names the culprit, `SENSOR (=n)`; menuconfig's `?` on BME280 shows the same in its dependency list. Add `CONFIG_SENSOR=y`.
3. Still false? Check `build/zephyr/zephyr.dts` (the final, merged devicetree). Your node says `status = "disabled"`, or the `compatible` string is misspelled, so no driver ever matched it.
4. It links, but `device_is_ready(bme)` is false at boot? Now the driver *is* in the image and its init failed: the chip didn't answer on the bus (wrong address, no power, bad wiring). That's a hardware question, not a build one.

### Gotchas

- **Pristine builds.** CMake caches the board and the config. After changing the board or overlay *file names*, run `west build -p always`.
- **The unmet-dependency warning is an error in disguise.** A typo or an out-of-range value stops the build, but a `=y` dropped for a missing dependency is only a warning: the image builds and the feature is missing at runtime. Read the Kconfig warnings at the top of the build log.
- **Don't edit `.config` by hand.** It's regenerated on every build.
- **Stack sizes live in Kconfig too** (`CONFIG_MAIN_STACK_SIZE`, `CONFIG_SYSTEM_WORKQUEUE_STACK_SIZE`). The kernel objects you'll meet next (threads, the workqueue) are configured here.
- **A heads-up for the next node:** Zephyr flips the Fundamentals priority rule. In `rtos.h` a bigger number was more urgent. In Zephyr a **smaller** number is more urgent, and **negative** numbers are cooperative.

### In the wild

- **Nordic's nRF Connect SDK** is a west manifest that imports Zephyr plus Nordic's own repos. Every nRF52/nRF53/nRF91 product (wearables, trackers, smart locks) is built this way. Teams pin the SDK tag in `west.yml` and bump it deliberately.
- **Product teams keep a "fragment per feature"**: `overlay-debug.conf`, `overlay-bt.conf`, `overlay-usb.conf`. CI builds the matrix (board × fragment) with `west build` or **Twister**, Zephyr's test runner.
- **Size budgets**: `west build -t rom_report` and `ram_report` show what each subsystem costs. They're the first stop when a feature doesn't fit in flash.
