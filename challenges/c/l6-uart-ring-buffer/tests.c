#include <stdbool.h>
#include <stddef.h>
#include <string.h>

static void init_dirty(RingBuffer *rb) {
    memset(rb, 0x5A, sizeof *rb);   // garbage, like an uninitialized local
    ring_init(rb);
}

TEST(new_buffer_is_empty) {
    RingBuffer rb;
    init_dirty(&rb);
    EXPECT_TRUE(ring_is_empty(&rb));
    EXPECT_FALSE(ring_is_full(&rb));
    int v = 99;
    EXPECT_FALSE(ring_pop(&rb, &v));
    EXPECT_EQ(v, 99);
}

TEST(values_come_out_in_order) {
    RingBuffer rb;
    init_dirty(&rb);
    EXPECT_TRUE(ring_push(&rb, 10));
    EXPECT_TRUE(ring_push(&rb, 20));
    EXPECT_TRUE(ring_push(&rb, 30));
    EXPECT_FALSE(ring_is_empty(&rb));
    int v = 0;
    EXPECT_TRUE(ring_pop(&rb, &v));
    EXPECT_EQ(v, 10);
    EXPECT_TRUE(ring_pop(&rb, &v));
    EXPECT_EQ(v, 20);
    EXPECT_TRUE(ring_pop(&rb, &v));
    EXPECT_EQ(v, 30);
    EXPECT_TRUE(ring_is_empty(&rb));
}

TEST(full_buffer_rejects_pushes_without_overwriting) {
    RingBuffer rb;
    init_dirty(&rb);
    for (int i = 1; i <= RING_CAP; i++) EXPECT_TRUE(ring_push(&rb, i));
    EXPECT_TRUE(ring_is_full(&rb));
    EXPECT_FALSE(ring_push(&rb, 999));
    int v = 0;
    EXPECT_TRUE(ring_pop(&rb, &v));
    EXPECT_EQ(v, 1);   // the oldest value survived
    EXPECT_FALSE(ring_is_full(&rb));
}

TEST(empty_pop_leaves_out_unchanged_after_draining) {
    RingBuffer rb;
    init_dirty(&rb);
    ring_push(&rb, 5);
    int v = 0;
    EXPECT_TRUE(ring_pop(&rb, &v));
    v = -1;
    EXPECT_FALSE(ring_pop(&rb, &v));
    EXPECT_EQ(v, -1);
}

TEST(indices_wrap_around_the_end) {
    RingBuffer rb;
    init_dirty(&rb);
    int v = 0;
    for (int i = 0; i < 3; i++) ring_push(&rb, i);
    for (int i = 0; i < 3; i++) ring_pop(&rb, &v);
    // head and tail are now near the end of the array: fill it across the seam
    for (int i = 100; i < 100 + RING_CAP; i++) EXPECT_TRUE(ring_push(&rb, i));
    EXPECT_TRUE(ring_is_full(&rb));
    EXPECT_FALSE(ring_push(&rb, -1));
    for (int i = 100; i < 100 + RING_CAP; i++) {
        EXPECT_TRUE(ring_pop(&rb, &v));
        EXPECT_EQ(v, i);
    }
    EXPECT_TRUE(ring_is_empty(&rb));
}

TEST(survives_a_long_stream_of_interleaved_traffic) {
    RingBuffer rb;
    init_dirty(&rb);
    int next_in = 0, next_out = 0, v = 0;
    for (int round = 0; round < 500; round++) {
        int pushes = round % 3 + 1, pops = (round + 1) % 3 + 1;
        for (int k = 0; k < pushes; k++) if (ring_push(&rb, next_in)) next_in++;
        for (int k = 0; k < pops; k++) {
            if (ring_pop(&rb, &v)) {
                EXPECT_EQ(v, next_out);
                next_out++;
            }
        }
    }
    EXPECT_TRUE(next_out > 900);
}
