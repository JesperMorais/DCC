#include <stdbool.h>
#include <stddef.h>

TEST(empty_history_has_length_zero) {
    EXPECT_EQ(history_length(NULL), 0);
    history_free(NULL);   // must be safe
}

TEST(push_puts_the_newest_step_first) {
    Step *h = NULL;
    EXPECT_TRUE(history_push(&h, 1));
    EXPECT_TRUE(history_push(&h, 2));
    EXPECT_TRUE(history_push(&h, 3));
    int seen[3] = {0, 0, 0};
    size_t i = 0;
    for (const Step *s = h; s != NULL && i < 3; s = s->next) seen[i++] = s->action;
    int expected[3] = {3, 2, 1};
    bool ends = h != NULL && h->next != NULL && h->next->next != NULL && h->next->next->next == NULL;
    history_free(h);
    EXPECT_INT_ARRAY_EQ(seen, expected, 3);
    EXPECT_TRUE(ends);
}

TEST(first_push_creates_a_one_node_list) {
    Step *h = NULL;
    EXPECT_TRUE(history_push(&h, 42));
    EXPECT_NOT_NULL(h);
    int action = h->action;
    bool alone = h->next == NULL;
    history_free(h);
    EXPECT_EQ(action, 42);
    EXPECT_TRUE(alone);
}

TEST(length_counts_every_node) {
    Step *h = NULL;
    for (int i = 0; i < 5; i++) history_push(&h, i);
    size_t len = history_length(h);
    history_free(h);
    EXPECT_EQ(len, 5);
}

TEST(free_releases_a_long_history) {
    Step *h = NULL;
    for (int i = 0; i < 1000; i++) {
        if (!history_push(&h, i)) break;
    }
    size_t len = history_length(h);
    history_free(h);   // the leak check runs after this test
    EXPECT_EQ(len, 1000);
}
