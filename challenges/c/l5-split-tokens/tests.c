#include <stddef.h>
#include <stdlib.h>
#include <string.h>

TEST(splits_a_simple_command) {
    size_t n = 99;
    char **t = split_tokens("ls -la /tmp", ' ', &n);
    EXPECT_NOT_NULL(t);
    EXPECT_EQ(n, 3);
    EXPECT_STR_EQ(t[0], "ls");
    EXPECT_STR_EQ(t[1], "-la");
    EXPECT_STR_EQ(t[2], "/tmp");
    EXPECT_NULL(t[3]);
    free_tokens(t);
}

TEST(extra_delimiters_make_no_empty_tokens) {
    size_t n = 99;
    char **t = split_tokens("  ls   -la /tmp ", ' ', &n);
    EXPECT_NOT_NULL(t);
    EXPECT_EQ(n, 3);
    EXPECT_STR_EQ(t[0], "ls");
    EXPECT_STR_EQ(t[1], "-la");
    EXPECT_STR_EQ(t[2], "/tmp");
    EXPECT_NULL(t[3]);
    free_tokens(t);
}

TEST(other_delimiters_work_too) {
    size_t n = 99;
    char **t = split_tokens("/usr/local//bin", '/', &n);
    EXPECT_NOT_NULL(t);
    EXPECT_EQ(n, 3);
    EXPECT_STR_EQ(t[0], "usr");
    EXPECT_STR_EQ(t[1], "local");
    EXPECT_STR_EQ(t[2], "bin");
    EXPECT_NULL(t[3]);
    free_tokens(t);
}

TEST(a_line_without_delimiters_is_one_token) {
    size_t n = 99;
    char **t = split_tokens("pwd", ' ', &n);
    EXPECT_NOT_NULL(t);
    EXPECT_EQ(n, 1);
    EXPECT_STR_EQ(t[0], "pwd");
    EXPECT_NULL(t[1]);
    free_tokens(t);
}

TEST(empty_or_blank_lines_give_just_the_null_entry) {
    size_t n = 99;
    char **t = split_tokens("", ' ', &n);
    EXPECT_NOT_NULL(t);
    EXPECT_EQ(n, 0);
    EXPECT_NULL(t[0]);
    free_tokens(t);
    n = 99;
    t = split_tokens("    ", ' ', &n);
    EXPECT_NOT_NULL(t);
    EXPECT_EQ(n, 0);
    EXPECT_NULL(t[0]);
    free_tokens(t);
    free_tokens(NULL);
}

TEST(each_token_is_its_own_allocation) {
    size_t n = 0;
    char **t = split_tokens("cat notes.txt", ' ', &n);
    EXPECT_NOT_NULL(t);
    EXPECT_EQ(n, 2);
    free(t[1]);                          // the caller replaces one argument
    t[1] = malloc(8);
    memcpy(t[1], "log.txt", 8);
    EXPECT_STR_EQ(t[0], "cat");
    EXPECT_STR_EQ(t[1], "log.txt");
    free_tokens(t);
}
