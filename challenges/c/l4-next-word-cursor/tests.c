#include <stdbool.h>
#include <stddef.h>
#include <string.h>

TEST(reads_the_first_word_and_moves_the_cursor) {
    const char *text = "led on";
    const char *cur = text;
    const char *w = NULL;
    size_t n = 0;
    EXPECT_TRUE(next_word(&cur, &w, &n));
    EXPECT_PTR_EQ(w, text);
    EXPECT_UEQ(n, 3);
    EXPECT_PTR_EQ(cur, text + 3);
}

TEST(skips_leading_spaces) {
    const char *text = "   reboot";
    const char *cur = text;
    const char *w = NULL;
    size_t n = 0;
    EXPECT_TRUE(next_word(&cur, &w, &n));
    EXPECT_PTR_EQ(w, text + 3);
    EXPECT_UEQ(n, 6);
    EXPECT_PTR_EQ(cur, text + 9);
}

TEST(loop_splits_a_whole_command) {
    const char *cur = "  led  set 3 ";
    const char *w = NULL;
    size_t n = 0;
    EXPECT_TRUE(next_word(&cur, &w, &n));
    EXPECT_TRUE(n == 3 && strncmp(w, "led", 3) == 0);
    EXPECT_TRUE(next_word(&cur, &w, &n));
    EXPECT_TRUE(n == 3 && strncmp(w, "set", 3) == 0);
    EXPECT_TRUE(next_word(&cur, &w, &n));
    EXPECT_TRUE(n == 1 && w[0] == '3');
    EXPECT_FALSE(next_word(&cur, &w, &n));
}

TEST(only_spaces_returns_false_and_parks_at_the_end) {
    const char *text = "    ";
    const char *cur = text;
    const char *w = NULL;
    size_t n = 0;
    EXPECT_FALSE(next_word(&cur, &w, &n));
    EXPECT_PTR_EQ(cur, text + 4);
}

TEST(empty_string_returns_false) {
    const char *text = "";
    const char *cur = text;
    const char *w = NULL;
    size_t n = 0;
    EXPECT_FALSE(next_word(&cur, &w, &n));
    EXPECT_PTR_EQ(cur, text);
}

TEST(stays_inside_an_exact_size_buffer) {
    char buf[4] = {' ', 'g', 'o', '\0'};
    const char *cur = buf;
    const char *w = NULL;
    size_t n = 0;
    EXPECT_TRUE(next_word(&cur, &w, &n));
    EXPECT_UEQ(n, 2);
    EXPECT_PTR_EQ(cur, buf + 3);
    EXPECT_FALSE(next_word(&cur, &w, &n));
    EXPECT_FALSE(next_word(&cur, &w, &n));
    EXPECT_PTR_EQ(cur, buf + 3);
}
