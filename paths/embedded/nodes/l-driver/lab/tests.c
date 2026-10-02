#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

/* ===== The kernel side, played by the tests ===== */

static unsigned long uaccess_budget = (unsigned long)-1;   /* bytes copy_to_user may copy before it "faults" */
static int kzalloc_fails;
static int signal_pending;                                   /* makes wait_for_samples() return -ERESTARTSYS */
static int wait_calls;

void *kzalloc(size_t size, int gfp) {
    (void)gfp;
    return kzalloc_fails ? NULL : calloc(1, size);
}

void kfree(const void *p) { free((void *)p); }

unsigned long copy_to_user(void __user *to, const void *from, unsigned long n) {
    unsigned long ok = n < uaccess_budget ? n : uaccess_budget;
    memcpy(to, from, ok);
    uaccess_budget -= ok;
    return n - ok;   /* bytes NOT copied */
}

int wait_for_samples(void) {
    wait_calls++;
    if (signal_pending) return -ERESTARTSYS;
    sensor_irq_push(777);   /* while we "slept", the sensor interrupt delivered a sample */
    return 0;
}

/* Open /dev/sensor the way the VFS does: a fresh struct file, then fops->open. */
static int vfs_open(struct inode *ino, struct file *f, unsigned flags) {
    *f = (struct file){ .f_flags = flags };
    return sensor_fops.open(ino, f);
}

static struct inode dev_sensor0 = { .i_minor = 0 };

/* ===== tests ===== */

TEST(open_sets_private_data_and_release_frees_it) {
    struct file f;
    EXPECT_EQ(vfs_open(&dev_sensor0, &f, O_RDONLY), 0);
    EXPECT_NOT_NULL(f.private_data);
    EXPECT_PTR_EQ(((struct sensor_reader *)f.private_data)->dev, &sensor0);
    EXPECT_EQ(sensor_fops.release(&dev_sensor0, &f), 0);   /* the leak check verifies the kfree */

    struct inode other = { .i_minor = 3 };
    EXPECT_EQ(vfs_open(&other, &f, O_RDONLY), -ENODEV);
    kzalloc_fails = 1;
    EXPECT_EQ(vfs_open(&dev_sensor0, &f, O_RDONLY), -ENOMEM);
}

TEST(read_returns_whole_samples_oldest_first) {
    struct file f;
    EXPECT_EQ(vfs_open(&dev_sensor0, &f, O_RDONLY), 0);
    sensor_irq_push(10);
    sensor_irq_push(20);
    sensor_irq_push(30);
    struct sensor_sample s[8];
    loff_t pos = 0;
    EXPECT_EQ(sensor_fops.read(&f, (char *)s, sizeof s, &pos), 3 * (long)sizeof(struct sensor_sample));
    EXPECT_EQ(s[0].seq, 0);
    EXPECT_EQ(s[0].value, 10);
    EXPECT_EQ(s[2].seq, 2);
    EXPECT_EQ(s[2].value, 30);

    for (int i = 0; i < 3; i++) sensor_irq_push(100 + i);
    /* 20 bytes of room = 2 whole samples. Never hand out half a sample. */
    EXPECT_EQ(sensor_fops.read(&f, (char *)s, 20, &pos), 16);
    EXPECT_EQ(s[0].value, 100);
    EXPECT_EQ(s[1].value, 101);
    EXPECT_EQ(sensor0.count, 1);
    EXPECT_EQ(sensor_fops.read(&f, (char *)s, 7, &pos), -EINVAL);   /* smaller than one sample */
    EXPECT_EQ(sensor0.count, 1);
    EXPECT_EQ(((struct sensor_reader *)f.private_data)->samples_read, 5);
    sensor_fops.release(&dev_sensor0, &f);
}

TEST(empty_device_eagain_when_nonblocking_otherwise_waits) {
    struct file nb, blk;
    EXPECT_EQ(vfs_open(&dev_sensor0, &nb, O_RDONLY | O_NONBLOCK), 0);
    EXPECT_EQ(vfs_open(&dev_sensor0, &blk, O_RDONLY), 0);
    struct sensor_sample s[4];
    loff_t pos = 0;
    EXPECT_EQ(sensor_fops.read(&nb, (char *)s, sizeof s, &pos), -EAGAIN);
    EXPECT_EQ(wait_calls, 0);                         /* O_NONBLOCK never sleeps */

    EXPECT_EQ(sensor_fops.read(&blk, (char *)s, sizeof s, &pos), 8);
    EXPECT_EQ(wait_calls, 1);                         /* slept once, woke up with data */
    EXPECT_EQ(s[0].value, 777);

    signal_pending = 1;                               /* Ctrl-C while blocked */
    EXPECT_EQ(sensor_fops.read(&blk, (char *)s, sizeof s, &pos), -ERESTARTSYS);
    sensor_fops.release(&dev_sensor0, &nb);
    sensor_fops.release(&dev_sensor0, &blk);
}

TEST(a_faulting_user_buffer_gives_efault_and_loses_nothing) {
    struct file f;
    EXPECT_EQ(vfs_open(&dev_sensor0, &f, O_RDONLY), 0);
    sensor_irq_push(1);
    sensor_irq_push(2);
    sensor_irq_push(3);
    struct sensor_sample s[3];
    loff_t pos = 0;
    uaccess_budget = 8;                               /* the user page ends after 8 bytes */
    EXPECT_EQ(sensor_fops.read(&f, (char *)s, sizeof s, &pos), -EFAULT);
    EXPECT_EQ(sensor0.count, 3);                      /* nothing consumed */
    uaccess_budget = (unsigned long)-1;
    EXPECT_EQ(sensor_fops.read(&f, (char *)s, sizeof s, &pos), 24);
    EXPECT_EQ(s[0].value, 1);
    EXPECT_EQ(s[2].value, 3);
    sensor_fops.release(&dev_sensor0, &f);
}

TEST(ioctl_count_reset_and_unknown_commands) {
    struct file f;
    EXPECT_EQ(vfs_open(&dev_sensor0, &f, O_RDONLY), 0);
    for (int i = 0; i < 5; i++) sensor_irq_push(i);
    int n = -1;
    EXPECT_EQ(sensor_fops.unlocked_ioctl(&f, SENSOR_IOC_COUNT, (unsigned long)&n), 0);
    EXPECT_EQ(n, 5);
    uaccess_budget = 0;
    EXPECT_EQ(sensor_fops.unlocked_ioctl(&f, SENSOR_IOC_COUNT, (unsigned long)&n), -EFAULT);
    uaccess_budget = (unsigned long)-1;
    EXPECT_EQ(sensor_fops.unlocked_ioctl(&f, SENSOR_IOC_RESET, 0), 0);
    EXPECT_EQ(sensor0.count, 0);
    EXPECT_EQ(sensor_fops.unlocked_ioctl(&f, _IO('x', 9), 0), -ENOTTY);
    sensor_irq_push(9);                               /* the ring still works after a reset */
    struct sensor_sample s;
    loff_t pos = 0;
    EXPECT_EQ(sensor_fops.read(&f, (char *)&s, sizeof s, &pos), 8);
    EXPECT_EQ(s.value, 9);
    sensor_fops.release(&dev_sensor0, &f);
}

TEST(each_open_file_has_its_own_state) {
    struct file a, b;
    EXPECT_EQ(vfs_open(&dev_sensor0, &a, O_RDONLY), 0);
    EXPECT_EQ(vfs_open(&dev_sensor0, &b, O_RDONLY), 0);
    EXPECT_TRUE(a.private_data != b.private_data);
    for (int i = 0; i < 20; i++) sensor_irq_push(i);  /* the ring holds 16: the IRQ drops 4 */
    struct sensor_sample s[16];
    loff_t pos = 0;
    EXPECT_EQ(sensor_fops.read(&a, (char *)s, 10 * sizeof s[0], &pos), 80);
    EXPECT_EQ(sensor_fops.read(&b, (char *)s, sizeof s, &pos), 48);
    EXPECT_EQ(s[5].value, 15);                        /* oldest first, across the wrap-around */
    EXPECT_EQ(((struct sensor_reader *)a.private_data)->samples_read, 10);
    EXPECT_EQ(((struct sensor_reader *)b.private_data)->samples_read, 6);
    sensor_fops.release(&dev_sensor0, &a);
    sensor_fops.release(&dev_sensor0, &b);
}
