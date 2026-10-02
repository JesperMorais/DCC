A field engineer once phoned in a "thermal runaway" on a fleet of outdoor gateways. The fans were stuck at 100 % on every unit. Nothing was overheating. The fan daemon had read `/sys/class/thermal/thermal_zone0/temp` once at boot. A kernel update had renumbered the zones, so zone 0 was now the *modem's* sensor, which reported an error, and the daemon treated "error" as "very hot". The fix was twenty lines: find the zone by its `type`, and parse the value as if it might lie to you.

This node is about those twenty lines.

### Everything is a file, one value per file

On a microcontroller you read a temperature by poking a register. On Linux, the kernel owns the hardware, and it exposes what you're allowed to see as **files**:

- **procfs** (`/proc`) is about *processes and the kernel*: `/proc/cpuinfo`, `/proc/meminfo`, `/proc/<pid>/status`. It's old and a bit of a junk drawer, and many of its files are multi-line reports.
- **sysfs** (`/sys`) is about *devices and drivers*. It's a live view of the kernel's device model, and it follows a strict rule: **one value per file**, as text, with a trailing newline.

```
$ cat /sys/class/thermal/thermal_zone0/type
x86_pkg_temp
$ cat /sys/class/thermal/thermal_zone0/temp
45123                       ← millidegrees: 45.123 °C
$ cat /sys/class/hwmon/hwmon2/temp1_input
51000
```

A single value has no format to parse and no ABI to break, and you can debug it with `cat` and `echo` from a shell. Units are integers in milli-units (m°C, mV, mA), because the kernel avoids floating point.

Compare it with Fundamentals: there, a register was a 32-bit number at a fixed address. Here, an "attribute" is a small text file at a path. The driver's `show()` callback prints the value at the moment you `read()`.

### Classes, and why you never hard-code a number

`/sys/class/<kind>/` groups devices by what they *are*, not where they sit on the bus: `thermal`, `hwmon`, `net`, `leds`, `power_supply`. The instance numbers (`thermal_zone3`, `hwmon2`) are handed out at **probe order**, which changes with kernel versions, device-tree edits and module load order. So:

1. List the class directory with `opendir`/`readdir`.
2. Match on an identifying attribute (`type` for thermal, `name` for hwmon).
3. Only then read the value.

`readdir()` returns entries in **no particular order**. If two zones match, decide explicitly which one wins.

### GPIO: the sysfs interface you should no longer use

For years, GPIO was the poster child of sysfs: `echo 17 > /sys/class/gpio/export`, then `echo out > gpio17/direction`. It's **deprecated**. Its pin numbers are global and unstable, nothing cleans up when your program crashes, and you can't change several lines atomically.

Modern code uses the **GPIO character device** `/dev/gpiochipN` through **libgpiod**. You request lines by chip and offset (or by name from the device tree), you get an fd, and the kernel releases the lines when that fd closes. Edge events arrive as readable data on that fd, which means you can wait for them with `poll`/`epoll` (next node).

```c
/* libgpiod v2, roughly */
struct gpiod_chip *chip = gpiod_chip_open("/dev/gpiochip0");
/* ... request offset 17 as output, then: */
gpiod_line_request_set_value(req, 17, GPIOD_LINE_VALUE_ACTIVE);
```

The rule of thumb: sysfs for **reading state and configuration** (hwmon, thermal, LEDs, backlight), and character devices for **I/O with ownership** (GPIO, SPI, I²C, serial).

### Worked example: reading an attribute robustly

A sysfs read should handle four things:

```c
int fd = open(path, O_RDONLY | O_CLOEXEC);
if (fd < 0) return -errno;            /* grab errno *now*: -ENOENT, -EACCES … */

/* read() may return fewer bytes than asked for. Loop until it returns 0 (EOF). */
ssize_t n = read(fd, buf + len, cap - 1 - len);

close(fd);                            /* on EVERY path, including errors */
```

Then strip the trailing `"\n"` and parse with `strtol(buf, &end, 10)`. The number is valid only if `end != buf` (some digits were read) and `*end == '\0'` (nothing after them). `atoi("42C")` happily returns 42, and `atoi("")` returns 0, which is a perfectly plausible temperature. That's how the gateway fans got stuck.

Return errors as **negative errno**, the kernel's own convention. The caller can then `switch` on `-ENOENT` ("sensor not present") vs `-EINVAL` ("sensor present but talking nonsense").

### Gotchas

- **Partial reads are legal.** Real sysfs usually gives you the whole value in one `read()`, but procfs, pipes and FUSE don't promise that. Code that does one `read()` works on your desk and fails in the field. The lab feeds you a value in two chunks.
- **Leaked fds are a slow death.** A daemon that polls a sensor once a second and leaks one fd on the error path hits `EMFILE` (usually 1024 fds) in about 17 minutes. After that, *every* `open()` in the process fails. Keep one `close()` at the bottom of the function and send all paths through it.
- **`errno` is fragile.** Any later libc call can overwrite it, so capture `-errno` immediately.
- **Some reads have side effects** or are slow. A hwmon read can trigger an I²C transaction that takes milliseconds, so don't call it in a tight loop.
- **You can't `epoll` a sysfs file.** epoll refuses regular files with `-EPERM`, and a regular file is always "readable" anyway. Drivers that support change notification call `sysfs_notify()`. You then `poll()` the open attribute for **`POLLPRI | POLLERR`**, and after it wakes you, `lseek(fd, 0, SEEK_SET)` and read the value again.

### In the wild

- **Fan and thermal daemons** (`thermald`, Raspberry Pi fan scripts, laptop `fancontrol`) are exactly this lab: scan `/sys/class/thermal` or `/sys/class/hwmon`, match by type or name, read millidegrees.
- **Battery gauges** on Android and phones read `/sys/class/power_supply/battery/capacity` and `voltage_now`. The UI shows the value straight from sysfs.
- **LED and backlight control** through `/sys/class/leds/*/brightness` and `/sys/class/backlight/*/brightness`, often from shell scripts in an initramfs.
- **Interview insight:** "What's the difference between sysfs and procfs?" A strong answer is that sysfs is the structured device model with one value per file and is a stable ABI per attribute, while procfs is process and kernel information, historically freeform. Bonus points for saying "and GPIO moved from sysfs to the gpiochip character device, because sysfs had no ownership or cleanup".
