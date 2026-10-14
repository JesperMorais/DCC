#include <stdbool.h>
#include <string.h>

TEST(inserting_at_the_end_appends) {
    EditLine l = {0};
    line_set(&l, "temp=");
    EXPECT_TRUE(line_insert(&l, 5, "21.5"));
    EXPECT_STR_EQ(l.text, "temp=21.5");
    EXPECT_EQ(l.len, 9);
}

TEST(inserting_in_the_middle_keeps_the_rest_of_the_line) {
    EditLine l = {0};
    line_set(&l, "helo");
    EXPECT_TRUE(line_insert(&l, 3, "l"));
    EXPECT_STR_EQ(l.text, "hello");
    EXPECT_EQ(l.len, 5);
}

TEST(inserting_at_the_start) {
    EditLine l = {0};
    line_set(&l, "ls -l");
    EXPECT_TRUE(line_insert(&l, 0, "sudo "));
    EXPECT_STR_EQ(l.text, "sudo ls -l");
}

TEST(erasing_clamps_at_the_end_of_the_line) {
    EditLine l = {0};
    line_set(&l, "reboot now");
    EXPECT_TRUE(line_erase(&l, 6, 4));
    EXPECT_STR_EQ(l.text, "reboot");
    EXPECT_TRUE(line_erase(&l, 2, 100));
    EXPECT_STR_EQ(l.text, "re");
    EXPECT_EQ(l.len, 2);
}

TEST(rejects_bad_positions_and_overflow) {
    EditLine l = {0};
    line_set(&l, "abc");
    EXPECT_FALSE(line_insert(&l, 4, "x"));
    EXPECT_FALSE(line_erase(&l, 4, 1));
    char big[LINE_CAP];
    for (int i = 0; i < LINE_CAP - 1; i++) big[i] = 'z';
    big[LINE_CAP - 1] = '\0';
    EXPECT_FALSE(line_insert(&l, 1, big));
    EXPECT_FALSE(line_replace(&l, 0, 1, big));
    EXPECT_STR_EQ(l.text, "abc");
    EXPECT_EQ(l.len, 3);
}

TEST(replace_swaps_a_value_in_the_middle) {
    EditLine l = {0};
    line_set(&l, "a=1;b=2");
    EXPECT_TRUE(line_replace(&l, 2, 1, "10"));
    EXPECT_STR_EQ(l.text, "a=10;b=2");
    EXPECT_EQ(l.len, 8);
}

TEST(fills_the_line_exactly_to_capacity) {
    EditLine l = {0};
    char dashes[LINE_CAP - 3];
    memset(dashes, '-', sizeof dashes - 1);
    dashes[sizeof dashes - 1] = '\0';
    line_set(&l, dashes);
    EXPECT_TRUE(line_insert(&l, l.len, "end"));
    EXPECT_EQ(l.len, LINE_CAP - 1);
    EXPECT_STR_EQ(l.text + LINE_CAP - 4, "end");
    EXPECT_FALSE(line_insert(&l, 0, ">"));
    EXPECT_EQ(l.text[0], '-');
}
