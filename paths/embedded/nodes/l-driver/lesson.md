You `cat /dev/ttyUSB0` and characters from your microcontroller scroll past. You haven't opened a serial port or configured a UART register. You've read a file. Somewhere in the kernel, a few hundred lines of C turned your `read()` into "take bytes out of the UART's ring buffer". That code is a **character driver**, and its heart is a table of function pointers.

### The idea: a device is a table of operations

When a program calls `open("/dev/sensor")`, the kernel looks at the device node's **major** number (which driver) and **minor** number (which instance), then calls that driver's `struct file_operations`:

```c
static const struct file_operations sensor_fops = {
    .owner          = THIS_MODULE,
    .open           = sensor_open,
    .read           = sensor_read,
    .write          = sensor_write,
    .unlocked_ioctl = sensor_ioctl,
    .release        = sensor_release,   // last close()
};
```

Every syscall on the file descriptor becomes a call through this table:

```
userspace          VFS                         your driver
open()   ───────▶  new struct file  ───────▶   .open(inode, filp)
read()   ───────▶  check fd, flags  ───────▶   .read(filp, buf, count, &pos)
ioctl()  ───────▶                   ───────▶   .unlocked_ioctl(filp, cmd, arg)
close()  ───────▶  last reference   ───────▶   .release(inode, filp)
```

The kernel calls a negative return value an error: `-EAGAIN` becomes `read()` returning -1 with `errno = EAGAIN`. That's the same "negative errno" convention you used in the sysfs and epoll labs.

### private_data: per-open state

Each `open()` creates a new `struct file`. It has a `void *private_data` field that belongs entirely to you. Typical drivers allocate a small per-open struct in `.open`, hang it off `filp->private_data`, and free it in `.release`. Every other op starts with:

```c
struct sensor_reader *r = filp->private_data;
```

Two programs that open the same device each get their own reader, with their own statistics and their own read position, while sharing one `sensor_dev`.

### The read path, worked through

Your accelerometer IRQ fills a 16-slot ring buffer, the same one you built in Fundamentals. A userspace `read(fd, buf, 64)` arrives. A careful driver goes through these steps:

1. **Validate.** If the buffer can't hold even one record, return `-EINVAL`. Never hand out half a sample.
2. **Nothing there?** If the file was opened with `O_NONBLOCK` (in `filp->f_flags`), return `-EAGAIN` immediately. That's what lets epoll-based programs work. Otherwise **sleep**:
   ```c
   if (wait_event_interruptible(dev->wq, dev->count > 0))
       return -ERESTARTSYS;   // a signal (Ctrl-C) woke us: let it be handled
   ```
3. **Copy out.** `buf` is a userspace address. It might be unmapped, or point into the kernel to trick you. You may **never** dereference it directly. You call `copy_to_user(buf, src, n)` instead, which returns the number of bytes it **couldn't** copy. Anything other than 0 means `-EFAULT`.
4. **Then consume.** Advance the ring only after the copy succeeded. If you consume first and the copy faults, the samples are gone and nobody ever saw them.
5. **Return the byte count.**

`ioctl` is the side door for anything that isn't a stream of bytes: "how many samples are buffered?", "reset", "set the rate". The command numbers are built with macros that encode a direction, a size and a magic letter, `_IO('s', 0)` and `_IOR('s', 1, int)`. They live in a **uapi** header that userspace includes too. An unknown command gets `-ENOTTY`: "inappropriate ioctl for device", a name left over from 1970s teletypes.

### What's different in real kernel space

The lab models all this in userspace. The real thing has some extra rules:

- **There's no libc.** No `printf` (you have `pr_info`), no `malloc` (you have `kmalloc`/`kzalloc`), no floating point. A bad pointer doesn't give you a segfault, it gives you a kernel oops, and maybe a dead machine.
- **GFP flags tell the allocator what it's allowed to do.** `GFP_KERNEL` may sleep to reclaim memory, which is fine in `.open`. In an interrupt handler or under a spinlock you may not sleep, so you use `GFP_ATOMIC`, or better, allocate everything up front.
- **Concurrency is real.** The IRQ handler writes the ring while `.read` runs on another CPU, so they share a `spin_lock_irqsave`. This is the Fundamentals critical section, for SMP. Process-context paths, such as two readers racing, can use a `mutex`. But never hold a spinlock across `copy_to_user`, because it may sleep on a page fault. That's why the read path copies the samples into a local buffer first.
- **Registration.** `module_init(sensor_init)` runs at `insmod` (or at boot), and `module_exit` runs at `rmmod`. Inside, you call `misc_register()` (the easy way, with a dynamic minor under major 10) or `alloc_chrdev_region()` + `cdev_add()` + `device_create()`. The kernel then emits a uevent, and **udev** (or mdev on BusyBox systems) creates `/dev/sensor` with the right permissions. You never `mknod` by hand.
- **`__user`** is a real annotation. The kernel's static checker, `sparse`, warns if you dereference a `__user` pointer or pass a kernel pointer where a user one is expected. Run `make C=1`.
- **Error paths unwind in reverse.** If init fails halfway, it must undo what it did so far. The kernel idiom is `goto err_free;` labels.

### Gotchas

- **Treating `copy_to_user`'s return as "bytes copied".** It's the opposite: 0 means success.
- **Returning `-EAGAIN` in blocking mode,** or sleeping in non-blocking mode. Check `O_NONBLOCK` on every read.
- **Swallowing `-ERESTARTSYS`.** If you loop back to sleep instead of returning it, Ctrl-C can't kill the reader.
- **Forgetting the kfree in `.release`.** Every open leaks a little memory, and after a week of a daemon reopening the device, the OOM killer shows up.

### In the wild

- **`/dev/ttyS*`, `/dev/input/event*`, `/dev/gpiochip*`, `/dev/i2c-*` and `/dev/watchdog`** are all character drivers with this exact shape. `evdev` is a per-open ring buffer of `struct input_event`, with `-EAGAIN` and `wait_event_interruptible`, almost line for line what you're about to write.
- **The IIO subsystem** (accelerometers, ADCs) gives each sensor a buffered char device, `/dev/iio:deviceN`. It's the same design, generalised.
- **Interview question:** "What happens if your driver dereferences a user pointer directly?" Answer: on a modern CPU with SMAP/PAN, it's an immediate oops. Without them, it's a security hole, because the user can pass a kernel address. Always use `copy_to_user`/`copy_from_user`.
