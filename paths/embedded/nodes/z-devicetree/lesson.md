### The LED on the wrong pin

Your board spin moves the status LED from P0.13 to P0.17. In a typical vendor SDK you'd grep for `13`, edit three `#define`s in two headers, miss the one in the bootloader, and ship. In Zephyr you change **one line in a devicetree overlay**. The C code (`gpio_pin_set_dt(&led, 1)`) doesn't change, because it never knew which pin the LED was on. That separation of *"what hardware exists"* from *"what the code does"* is the devicetree.

### What a devicetree is

A devicetree is a text description of the hardware: a tree of **nodes**, each with **properties**. It's the same format Linux uses on ARM, but in Zephyr it's compiled into C macros at **build time**. There's no parsing at runtime and no cost in flash.

```dts
/ {
    aliases {
        led0 = &status_led;            /* a portable, stable name */
    };
    chosen {
        zephyr,console = &uart0;       /* "the" console, picked by the board */
    };
    leds {
        compatible = "gpio-leds";
        status_led: led_0 {           /* label: node-name */
            gpios = <&gpio0 13 GPIO_ACTIVE_LOW>;
        };
    };
};

&i2c0 {                                /* reference an existing node by label */
    status = "okay";
    clock-frequency = <I2C_BITRATE_FAST>;
    bme280@76 {                        /* unit address = I2C address */
        compatible = "bosch,bme280";
        reg = <0x76>;
    };
};
```

The vocabulary:

- **Node:** `bme280@76`. The `@76` is the unit address, which is the bus address for I2C.
- **Label:** `status_led:`. It's a handle for referencing the node from elsewhere (`&status_led`), and the name C code uses with `DT_NODELABEL(status_led)`.
- **`compatible`:** *"which binding describes me, and which driver handles me"*. It's a `"vendor,device"` string.
- **`status`:** `"okay"` means the node exists and its driver should be instantiated. `"disabled"` means it's present on the SoC but unused. Many SoC peripherals are disabled by default, and your board or overlay enables the ones you use.
- **`aliases` and `chosen`:** indirections. `led0` and `sw0` are the portable names that samples use. `chosen` picks system-wide roles such as the console, the shell UART or the code partition.

### Bindings: the schema

The devicetree compiler doesn't know what a `bme280` is. A **binding**, a YAML file matched by `compatible`, declares which properties are required and what their types are:

```yaml
# dts/bindings/sensor/bosch,bme280-i2c.yaml
compatible: "bosch,bme280"
include: [sensor-device.yaml, i2c-device.yaml]
properties:
  int-gpios:
    type: phandle-array
    description: optional data-ready interrupt
```

If a required property is missing, or a value has the wrong type, the **build fails** with a message that names the node. Your own custom hardware gets its own binding in `dts/bindings/` in your app or module.

### From the tree to C

Every node and property becomes a macro. You never see the generated names, because you use accessors:

```c
#include <zephyr/drivers/gpio.h>

#define LED_NODE DT_ALIAS(led0)                  /* or DT_NODELABEL(status_led) */
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED_NODE, gpios);

static const struct device *bme = DEVICE_DT_GET(DT_NODELABEL(bme280));
/* or: DEVICE_DT_GET_ONE(bosch_bme280) for "the one bme280" */

int main(void) {
    if (!gpio_is_ready_dt(&led) || !device_is_ready(bme)) {
        return -ENODEV;                          /* driver missing or failed init */
    }
    gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
    gpio_pin_set_dt(&led, 1);                    /* logical "on": ACTIVE_LOW handled */
}
```

- `GPIO_DT_SPEC_GET(node, gpios)` bundles the port device, pin number and flags (active low, pull-up) into one struct. Polarity lives in the devicetree, so the code says "on" and means it.
- `DEVICE_DT_GET(node)` gives you the driver instance as a compile-time constant. **If the node doesn't exist or isn't `okay`, it fails at link time**, not with a NULL at 3 a.m.
- **`device_is_ready()`** is the runtime half: the driver exists, but its init might have failed (a sensor not answering on the bus, a regulator not up). Check it once at startup.

### Overlays: changing the board without editing it

The board's `.dts` belongs to the board. Your PCB changes go in `boards/<board>.overlay`, which is merged on top:

```dts
/* boards/nrf52840dk_nrf52840.overlay */
&status_led {
    gpios = <&gpio0 17 GPIO_ACTIVE_LOW>;    /* the LED moved: one line */
};
&i2c0 {
    status = "okay";
    bme280@76 { compatible = "bosch,bme280"; reg = <0x76>; };
};
&uart1 { status = "disabled"; };            /* free those pins, save power */
```

After a build, `build/zephyr/zephyr.dts` contains the **fully merged** tree. When something is wrong, that file is the source of truth.

### Gotchas

- **The node name isn't the label.** `DT_NODELABEL()` takes the label (before the colon), with dashes turned into underscores: `my-sensor:` becomes `DT_NODELABEL(my_sensor)`.
- **`compatible` must match a binding exactly**, including the vendor prefix. A typo means no driver, and `DEVICE_DT_GET` then fails to link.
- **A disabled parent disables its children.** A sensor under `&i2c1` does nothing if `i2c1` is `"disabled"`.
- **Devicetree describes hardware, Kconfig enables software.** You usually need both: the node `okay` *and* the driver's Kconfig (which often defaults to `y` once an enabled node exists).
- **Pin control:** on modern Zephyr, pin muxing is in the tree too (`pinctrl-0`, `pinctrl-names`). Moving a UART to new pins is an overlay change to its pinctrl group.

### In the wild

- **One firmware, many boards.** Product companies keep one app and a `boards/` directory with an overlay per hardware revision. CI builds them all.
- **Nordic, ST, NXP and Espressif** ship board files and bindings upstream, so `west build -b <board>` for a dev kit works on day one, and your custom board is a small diff against it.
- **Linux developers feel at home.** It's the same syntax and the same mental model as Linux on ARM, with the difference that Zephyr resolves it all at compile time.
