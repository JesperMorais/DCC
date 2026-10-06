### Somebody wrote the BME280 driver

So far you've been on the **consumer** side: `DEVICE_DT_GET()`, `sensor_sample_fetch()`, `sensor_channel_get()`, and a sensor that just works. Every one of those devices exists because someone wrote the **provider** side: a driver that turns devicetree nodes into `struct device`s. When your company's board has a part nobody upstream supports, that someone is you. It sounds like kernel wizardry; it's mostly a page of plumbing around a few functions, and once you've read one, you can read the 300 in `zephyr/drivers/sensor/`.

### Two sides of one call

```
 consumer (your app)                      provider (the driver)
 sensor_sample_fetch(dev) ──▶ dev->api->sample_fetch(dev, chan) ──▶ wobble_fetch()
 sensor_channel_get(dev, ..) ─▶ dev->api->channel_get(...)       ──▶ wobble_get()
                                     │
              dev->config (read-only, from the devicetree)
              dev->data   (mutable state, one per instance)
```

A `struct device` is four pointers that matter: its name, its **API struct** (the functions), its **config** and its **data**. The consumer never sees your functions; it calls the subsystem's API and the subsystem jumps through `dev->api`. That's why any sensor driver works with any app that speaks the sensor API.

### Anatomy of an out-of-tree driver

The example is a pretend humidity sensor that wobbles around a value, in an app outside the Zephyr tree:

```
dts/bindings/vnd,wobble-hum.yaml   the binding: compatible + properties
app.overlay                        nodes that use it
src/wobble_hum.c                   the driver
prj.conf                           CONFIG_SENSOR=y
```

Zephyr searches your app's `dts/bindings/` automatically. The binding declares `compatible: "vnd,wobble-hum"` and two required `int` properties, `center-millipercent` and `wobble-millipercent`. The overlay adds two nodes:

```dts
room_hum: room-humidity {
	compatible = "vnd,wobble-hum";
	center-millipercent = <45000>;
	wobble-millipercent = <1500>;
};
```

and a second one, `cellar_hum`, at 80000 ± 500.

### The driver

```c
#define DT_DRV_COMPAT vnd_wobble_hum	/* "vnd,wobble-hum", lowercase, _ for , and - */

struct wobble_config { int32_t center_mp, wobble_mp; };	/* read-only */
struct wobble_data { int32_t last_mp; bool up; };		/* mutable */

static int wobble_fetch(const struct device *dev, enum sensor_channel chan)
{
	const struct wobble_config *cfg = dev->config;
	struct wobble_data *data = dev->data;

	if (chan != SENSOR_CHAN_ALL && chan != SENSOR_CHAN_HUMIDITY) {
		return -ENOTSUP;
	}
	data->last_mp = cfg->center_mp + (data->up ? cfg->wobble_mp : -cfg->wobble_mp);
	data->up = !data->up;
	return 0;
}

static int wobble_get(const struct device *dev, enum sensor_channel chan, struct sensor_value *val)
{
	const struct wobble_data *data = dev->data;

	if (chan != SENSOR_CHAN_HUMIDITY) {
		return -ENOTSUP;
	}
	return sensor_value_from_milli(val, data->last_mp);	/* 43500 -> 43, 500000 */
}

static int wobble_init(const struct device *dev)
{
	const struct wobble_config *cfg = dev->config;

	return cfg->center_mp <= 100000 ? 0 : -EINVAL;	/* not ready */
}

static DEVICE_API(sensor, wobble_api) = {
	.sample_fetch = wobble_fetch,
	.channel_get = wobble_get,
};

#define WOBBLE_DEFINE(inst)                                                  \
	static struct wobble_data wobble_data_##inst;                        \
	static const struct wobble_config wobble_config_##inst = {           \
		.center_mp = DT_INST_PROP(inst, center_millipercent),        \
		.wobble_mp = DT_INST_PROP(inst, wobble_millipercent),        \
	};                                                                   \
	DEVICE_DT_INST_DEFINE(inst, wobble_init, NULL, &wobble_data_##inst,  \
			      &wobble_config_##inst, POST_KERNEL,            \
			      CONFIG_SENSOR_INIT_PRIORITY, &wobble_api);

DT_INST_FOREACH_STATUS_OKAY(WOBBLE_DEFINE)
```

(The includes, `errno.h`, `zephyr/device.h` and `zephyr/drivers/sensor.h`, are left out here.) Read it from the bottom:

- **`DT_INST_FOREACH_STATUS_OKAY(WOBBLE_DEFINE)`** expands the macro once for every enabled node whose compatible matches `DT_DRV_COMPAT`, with `inst` = 0, 1, ... Two nodes, two expansions.
- Each expansion defines **its own** data and config (`##inst` pastes the number into the names) and fills the config with **`DT_INST_PROP(inst, center_millipercent)`**: the property name with dashes turned into underscores. That's a compile-time constant; nothing reads the devicetree at runtime.
- **`DEVICE_DT_INST_DEFINE`** creates the `struct device` for that node: init function, power management (`NULL`), data, config, **init level** and priority, API. `POST_KERNEL` runs init once the kernel is up; `CONFIG_SENSOR_INIT_PRIORITY` orders it among the other sensors.
- **`DEVICE_API(sensor, wobble_api)`** is the Zephyr 4.x way to declare a sensor API struct, so the build can check that a device really is a sensor.
- **Init** returns 0 or a negative errno. An error doesn't crash anything: the device stays in the image, and `device_is_ready()` returns false. That's where you reject a configuration that can't work.
- **`struct sensor_value`** is whole units in `val1` and millionths in `val2`, both with the same sign: 43.5 % is `{43, 500000}` and -1.25 °C is `{-1, -250000}`. `sensor_value_from_milli()` does the arithmetic.

### Running it

The consumer is ordinary app code: `DEVICE_DT_GET(DT_ALIAS(room_humidity))`, check `device_is_ready()`, fetch, get, print. On `native_sim`:

```
*** Booting Zephyr OS build v4.2.0 ***
room-humidity: 43.500000 %
room-humidity: 46.500000 %
cellar-humidity: 79.500000 %
cellar-humidity: 80.500000 %
```

Two instances, each wobbling on its own, because each has its own `data`. Change the cellar's center to 120000 and its init fails:

```
cellar-humidity is not ready
```

Leave `wobble_hum.c` out of `CMakeLists.txt` and the **link** fails instead:

```
src/main.c:25:(.text.main+0x2): undefined reference to `__device_dts_ord_18'
```

The node exists, so `DEVICE_DT_GET` compiled, but no driver defined the device. The number is the node's position in the tree; the comment at the top of `build/zephyr/include/generated/zephyr/devicetree_generated.h` lists them (`18  /room-humidity`).

### Gotchas

- **`DT_DRV_COMPAT` comes first**, before any `DT_INST_*` macro, and it's the compatible with every `,` and `-` turned into `_`. A typo there isn't an error: the FOREACH finds no nodes, gcc warns that `wobble_init` is *defined but not used*, and you get the link error above. That warning is the clue.
- **Config is `const`, data isn't.** Anything that changes at runtime goes in data, or two instances end up sharing it (a `static` variable in the driver file is shared by all of them).
- **Fetch and get are two calls.** If two threads use one device, one can fetch between the other's fetch and get. The driver can only protect each call; a consumer that needs the pair consistent locks around both.
- **Return `-ENOTSUP`** for channels you don't have. Consumers rely on that to probe what a sensor supports.

### In the wild

- Open `zephyr/drivers/sensor/bosch/bme280/bme280.c`: the same skeleton, with I2C or SPI reads in fetch and compensation maths in get. Most sensor drivers upstream follow it.
- Product teams keep their own drivers **out of tree**, in a Zephyr module or the app's own `drivers/` folder, with bindings in `dts/bindings/`, exactly as here. Upstreaming later is mostly moving files.
- **Emulated and fake drivers** like this one are real practice too: Zephyr's own tests use `zephyr,fake-...` devices and I2C emulators so driver consumers can be tested on `native_sim` without hardware.
