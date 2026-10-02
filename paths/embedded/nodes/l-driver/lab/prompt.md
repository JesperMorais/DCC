Your board has an accelerometer whose interrupt handler drops samples into a 16-slot ring buffer. Userspace wants to `read()` them from `/dev/sensor`. Write the driver's **`file_operations`**.

There's no kernel here, so the top of the starter is a tiny model of one: `struct file`, `struct inode`, `struct file_operations`, `kzalloc`/`kfree`, `copy_to_user`, and `wait_for_samples()`, which stands in for `wait_event_interruptible`. **The tests play the VFS.** They build a `struct file`, then call `sensor_fops.open`, `.read`, `.unlocked_ioctl` and `.release`, exactly like the kernel does when a program calls `open(2)`, `read(2)` and so on. The IRQ side (`sensor_irq_push`) is already written.

Errors are returned as **negative errno**, which libc turns into `-1` with `errno` set.

**`open(inode, filp)`**
- Only minor 0 exists, so any other minor gives `-ENODEV`.
- Allocate a `struct sensor_reader` with `kzalloc(…, GFP_KERNEL)`. If that fails, return `-ENOMEM`.
- Set `->dev = &sensor0` and store the reader in `filp->private_data`. Each open file gets its own reader.

**`read(filp, buf, count, ppos)`** hands out **whole samples**, oldest first.
- If `count` is smaller than one `struct sensor_sample` (8 bytes), return `-EINVAL`.
- If the buffer is empty and `O_NONBLOCK` is set in `filp->f_flags`, return `-EAGAIN`. If it's empty in blocking mode, call `wait_for_samples()` and check again. If the wait returns `-ERESTARTSYS` (a signal arrived), return that.
- Copy `min(count / 8, buffered)` samples with **`copy_to_user`**, which returns the number of bytes it **couldn't** copy. If it returns anything other than 0, return `-EFAULT` and **consume nothing**.
- On success, remove the samples from the ring, add them to the reader's `samples_read`, and return the number of bytes copied.

**`unlocked_ioctl(filp, cmd, arg)`**
- `SENSOR_IOC_COUNT`: copy the number of buffered samples (as an `int`) to the user pointer `arg`. If the copy fails, return `-EFAULT`.
- `SENSOR_IOC_RESET`: discard every buffered sample.
- Any other command gives `-ENOTTY`.

**`release(inode, filp)`** frees the reader. The leak check is on.

```
3 samples buffered, read(64 bytes)  → 24
read(20 bytes)                      → 16 (two whole samples, never half of one)
empty + O_NONBLOCK                  → -EAGAIN
user buffer faults after 8 bytes    → -EFAULT, all 3 samples still buffered
```
