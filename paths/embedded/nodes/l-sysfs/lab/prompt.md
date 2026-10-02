Your fan-control daemon has to find the CPU's thermal zone and read its temperature. On the target that means `/sys/class/thermal/thermal_zone*/`. Here the tests build the same tree under a root directory in the current working directory, for example `t5/sys/class/thermal/thermal_zone3/temp`, so you pass that root in.

```c
int sysfs_read_string(const char *path, char *buf, size_t cap);
int sysfs_read_long(const char *path, long *out);
int thermal_find_zone(const char *root, const char *type, long *temp_mc);
```

Use the real system calls: `open`, `read`, `close`, `opendir`, `readdir` and `closedir`. Errors come back as **negative errno** values, the same convention the kernel uses.

**`sysfs_read_string`** reads the whole file into `buf` and strips trailing whitespace, including the `\n` that sysfs always appends. It returns the length of the stripped string.
- If the file doesn't exist, it returns `-ENOENT` (and in general, whatever `-errno` the failing call reported).
- If the content doesn't fit in `cap - 1` bytes, it returns `-ERANGE`.
- `read()` can return **less than you asked for**. Keep reading until it returns 0.

**`sysfs_read_long`** parses a file that holds one decimal integer, such as `"45123\n"` or `"  -5000 \n"`. Leading and trailing whitespace are fine.
- It returns 0 and sets `*out`.
- Empty files, words, and numbers followed by junk (`"42C"`) give `-EINVAL`.
- On any error, `*out` is left untouched.

**`thermal_find_zone`** scans `<root>/sys/class/thermal/` for directories named `thermal_zone<N>` and ignores everything else (such as `cooling_device0`). It finds the zone whose `type` file equals `type`, reads that zone's `temp` into `*temp_mc` (in millidegrees Celsius), and returns `N`.
- If several zones match, the **lowest N** wins. `readdir()` returns entries in no particular order.
- If the directory is missing or no zone matches, it returns `-ENOENT`. If the matching zone's `temp` can't be read, it returns that error.

```c
sysfs_read_long("t1/sys/class/thermal/thermal_zone0/temp", &v)  // → 0, v = 45123
sysfs_read_long("t3/junk", &v)                                   // "42C\n" → -EINVAL
thermal_find_zone("t5", "cpu-thermal", &t)                       // → 3, t = 68000
thermal_find_zone("t5", "gpu-thermal", &t)                       // → -ENOENT
```

**No leaks:** a daemon calls these functions millions of times. Every `open` needs a `close` and every `opendir` needs a `closedir`, **including on the error paths**. One test counts the open file descriptors before and after 1,500 calls.
