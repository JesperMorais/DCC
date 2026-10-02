#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <sys/types.h>

/* ===== A tiny "kernel": these declarations mirror <linux/fs.h> and <linux/uaccess.h>. =====
 * The test harness plays the VFS and provides the functions. Don't change this part. */
#define __user                         /* marks pointers into user space (checked by sparse) */
#define GFP_KERNEL 0
#define ERESTARTSYS 512                /* "restart the syscall after the signal" */

struct inode { unsigned int i_minor; };
struct file  { unsigned int f_flags; void *private_data; };

struct file_operations {
    int     (*open)(struct inode *inode, struct file *filp);
    ssize_t (*read)(struct file *filp, char __user *buf, size_t count, loff_t *ppos);
    long    (*unlocked_ioctl)(struct file *filp, unsigned int cmd, unsigned long arg);
    int     (*release)(struct inode *inode, struct file *filp);
};

void *kzalloc(size_t size, int gfp);   /* NULL when out of memory */
void  kfree(const void *p);
/* Returns the number of bytes it could NOT copy (0 = success). */
unsigned long copy_to_user(void __user *to, const void *from, unsigned long n);
/* Stand-in for wait_event_interruptible(): sleeps until the IRQ adds samples.
 * Returns 0 when woken, or -ERESTARTSYS if a signal interrupted the sleep. */
int wait_for_samples(void);

/* ===== The device: a ring buffer filled by the sensor's interrupt handler. ===== */
#define SENSOR_RING 16
#define SENSOR_IOC_RESET _IO('s', 0)          /* discard every buffered sample */
#define SENSOR_IOC_COUNT _IOR('s', 1, int)    /* copy the number of buffered samples to *(int *)arg */

struct sensor_sample { uint32_t seq; int32_t value; };

struct sensor_dev {
    struct sensor_sample ring[SENSOR_RING];
    unsigned head;       /* oldest sample */
    unsigned count;      /* buffered samples */
    uint32_t next_seq;
    unsigned dropped;    /* samples lost because the ring was full */
};

struct sensor_dev sensor0;   /* minor 0: the only device */

/* Per-open state, hung off filp->private_data. */
struct sensor_reader {
    struct sensor_dev *dev;
    unsigned long samples_read;
};

/* The "IRQ handler" (already written): append a sample, or drop it if the ring is full. */
void sensor_irq_push(int32_t value) {
    struct sensor_dev *d = &sensor0;
    uint32_t seq = d->next_seq++;
    if (d->count == SENSOR_RING) {
        d->dropped++;
        return;
    }
    d->ring[(d->head + d->count) % SENSOR_RING] = (struct sensor_sample){ .seq = seq, .value = value };
    d->count++;
}

/* ===== Your driver ===== */

static int sensor_open(struct inode *inode, struct file *filp) {
    if (inode->i_minor != 0) return -ENODEV;
    struct sensor_reader *r = kzalloc(sizeof *r, GFP_KERNEL);
    if (!r) return -ENOMEM;
    r->dev = &sensor0;
    filp->private_data = r;
    return 0;
}

static ssize_t sensor_read(struct file *filp, char __user *buf, size_t count, loff_t *ppos) {
    struct sensor_reader *r = filp->private_data;
    struct sensor_dev *dev = r->dev;
    (void)ppos;   /* a stream device has no file position */

    if (count < sizeof(struct sensor_sample)) return -EINVAL;
    while (dev->count == 0) {
        if (filp->f_flags & O_NONBLOCK) return -EAGAIN;
        int rc = wait_for_samples();
        if (rc) return rc;   /* -ERESTARTSYS: let the signal be handled */
    }

    size_t n = count / sizeof(struct sensor_sample);
    if (n > dev->count) n = dev->count;
    struct sensor_sample out[SENSOR_RING];
    for (size_t i = 0; i < n; i++) out[i] = dev->ring[(dev->head + i) % SENSOR_RING];

    /* Copy first, consume after: a bad user pointer must not lose samples. */
    if (copy_to_user(buf, out, n * sizeof out[0])) return -EFAULT;
    dev->head = (unsigned)((dev->head + n) % SENSOR_RING);
    dev->count -= (unsigned)n;
    r->samples_read += n;
    return (ssize_t)(n * sizeof out[0]);
}

static long sensor_ioctl(struct file *filp, unsigned int cmd, unsigned long arg) {
    struct sensor_reader *r = filp->private_data;
    struct sensor_dev *dev = r->dev;
    switch (cmd) {
    case SENSOR_IOC_RESET:
        dev->head = dev->count = 0;
        return 0;
    case SENSOR_IOC_COUNT: {
        int n = (int)dev->count;
        return copy_to_user((void __user *)arg, &n, sizeof n) ? -EFAULT : 0;
    }
    default:
        return -ENOTTY;   /* "inappropriate ioctl for device" */
    }
}

static int sensor_release(struct inode *inode, struct file *filp) {
    (void)inode;
    kfree(filp->private_data);
    filp->private_data = NULL;
    return 0;
}

const struct file_operations sensor_fops = {
    .open = sensor_open,
    .read = sensor_read,
    .unlocked_ioctl = sensor_ioctl,
    .release = sensor_release,
};
