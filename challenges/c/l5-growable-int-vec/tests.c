#include <stdbool.h>
#include <stddef.h>

TEST(init_makes_an_empty_vector_without_allocating) {
    IntVec v = { (int *)&v, 99, 99 };   // garbage, like an uninitialized local
    intvec_init(&v);
    EXPECT_NULL(v.data);
    EXPECT_EQ(v.len, 0);
    EXPECT_EQ(v.cap, 0);
}

TEST(push_then_get) {
    IntVec v;
    intvec_init(&v);
    EXPECT_TRUE(intvec_push(&v, 10));
    EXPECT_TRUE(intvec_push(&v, 20));
    EXPECT_TRUE(intvec_push(&v, -5));
    int a = intvec_get(&v, 0), b = intvec_get(&v, 1), c = intvec_get(&v, 2);
    size_t len = v.len;
    intvec_free(&v);
    EXPECT_EQ(len, 3);
    EXPECT_EQ(a, 10);
    EXPECT_EQ(b, 20);
    EXPECT_EQ(c, -5);
}

TEST(capacity_starts_at_4_and_doubles) {
    IntVec v;
    intvec_init(&v);
    size_t caps[17];
    for (int i = 0; i < 17; i++) {
        intvec_push(&v, i);
        caps[i] = v.cap;
    }
    intvec_free(&v);
    EXPECT_EQ(caps[0], 4);    // after 1 push
    EXPECT_EQ(caps[3], 4);    // after 4 pushes: full, but no need to grow yet
    EXPECT_EQ(caps[4], 8);    // the 5th push grows
    EXPECT_EQ(caps[8], 16);   // the 9th push grows
    EXPECT_EQ(caps[16], 32);  // the 17th push grows
}

TEST(holds_a_thousand_samples) {
    IntVec v;
    intvec_init(&v);
    bool ok = true;
    for (int i = 0; i < 1000; i++) ok = ok && intvec_push(&v, i * 3);
    bool values_ok = v.len == 1000;
    for (size_t i = 0; values_ok && i < 1000; i++) values_ok = intvec_get(&v, i) == (int)i * 3;
    size_t len = v.len, cap = v.cap;
    intvec_free(&v);
    EXPECT_TRUE(ok);
    EXPECT_EQ(len, 1000);
    EXPECT_EQ(cap, 1024);
    EXPECT_TRUE(values_ok);
}

TEST(free_resets_and_the_vector_can_be_reused) {
    IntVec v;
    intvec_init(&v);
    for (int i = 0; i < 10; i++) intvec_push(&v, i);
    intvec_free(&v);
    bool reset = v.data == NULL && v.len == 0 && v.cap == 0;
    intvec_push(&v, 7);
    int first = v.len == 1 ? intvec_get(&v, 0) : -1;
    size_t cap = v.cap;
    intvec_free(&v);
    EXPECT_TRUE(reset);
    EXPECT_EQ(first, 7);
    EXPECT_EQ(cap, 4);
}

TEST(freeing_an_empty_vector_is_fine) {
    IntVec v;
    intvec_init(&v);
    intvec_free(&v);
    EXPECT_NULL(v.data);
    EXPECT_EQ(v.len, 0);
}
