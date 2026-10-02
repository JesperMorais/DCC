#include <zephyr/kernel.h>

/* One accelerometer interrupt per tick, from `first` to `last` inclusive. */
static uint32_t imu_last;
static void imu_line(void) {
    if (k_uptime_get_32() <= imu_last) imu_isr();
}
static void imu_samples(uint32_t first, uint32_t last) {
    imu_last = last;
    sim_irq_every(first, 1, "imu", imu_line);
}

static int32_t expected_sum(uint32_t from, uint32_t to) {
    int32_t sum = 0;
    for (uint32_t seq = from; seq <= to; seq++) sum += sample_value(seq);
    return sum;
}

TEST(every_sample_arrives_under_normal_load) {
    imu_samples(1, 50);
    sim_run(60);
    EXPECT_EQ(produced, 50);
    EXPECT_EQ(stats.consumed, 50);
    EXPECT_EQ(dropped, 0);
    EXPECT_EQ(stats.last_seq, 50);
    EXPECT_EQ(stats.sum_x, expected_sum(1, 50));
    EXPECT_EQ(stats.stale, 0);
}

TEST(a_full_queue_drops_samples_and_counts_them) {
    imu_samples(1, 59);
    sim_irq_at(10, "radio", radio_isr); /* fusion gets no CPU from 10 to 30 */
    sim_run(70);
    EXPECT_EQ(produced, 59);
    EXPECT_TRUE(dropped >= 10);
    /* everything is either consumed or counted as dropped: nothing vanishes */
    EXPECT_EQ(stats.consumed + dropped, produced);
    EXPECT_EQ(k_msgq_num_used_get(&imu_q), 0);
}

TEST(overflow_keeps_the_newest_samples) {
    imu_samples(1, 59);
    sim_irq_at(10, "radio", radio_isr);
    sim_run(70);
    /* At tick 10 the kernel hands sample #10 straight to the waiting fusion thread,
     * then the radio preempts it: that one sample is unavoidably stale. With drop-oldest,
     * everything still in the queue after the stall is at most 8 ms old. With
     * drop-newest, fusion would chew through 8 samples from 20 ms ago. */
    EXPECT_TRUE(stats.stale <= 1);
    EXPECT_EQ(stats.last_seq, 59);
}

TEST(the_isr_never_blocks_even_when_the_queue_is_full) {
    imu_samples(1, 99);
    sim_irq_at(5, "radio", radio_isr);
    sim_irq_at(40, "radio", radio_isr);
    sim_run(120);
    EXPECT_EQ(produced, 99);
    EXPECT_EQ(stats.consumed + dropped, produced);
    EXPECT_EQ(stats.last_seq, 99);
    EXPECT_TRUE(stats.stale <= 2);
}

TEST(sums_only_what_was_consumed) {
    imu_samples(1, 59);
    sim_irq_at(10, "radio", radio_isr);
    sim_run(70);
    /* the sample in hand (#10), the 8 newest from the stall (#23..#30), then #31..#59 */
    EXPECT_EQ(stats.sum_x, expected_sum(1, 10) + expected_sum(23, 59));
}
