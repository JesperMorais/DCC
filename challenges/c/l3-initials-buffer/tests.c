#include <stdbool.h>
#include <stdlib.h>

TEST(uppercases_the_first_letter_of_each_word) {
    char out[8];
    EXPECT_TRUE(initials("ada lovelace", out, sizeof out));
    EXPECT_STR_EQ(out, "AL");
}

TEST(multiple_spaces_between_words) {
    char out[8];
    EXPECT_TRUE(initials("  Grace   Brewster Hopper ", out, sizeof out));
    EXPECT_STR_EQ(out, "GBH");
}

TEST(exact_fit_uses_every_byte) {
    char *out = malloc(4);
    bool ok = initials("Grace Brewster Hopper", out, 4);
    char copy[4] = {out[0], out[1], out[2], out[3]};
    free(out);
    EXPECT_TRUE(ok);
    EXPECT_STR_EQ(copy, "GBH");
}

TEST(one_byte_too_small_returns_false_without_overflowing) {
    char *out = malloc(3);
    bool ok = initials("Grace Brewster Hopper", out, 3);
    free(out);
    EXPECT_FALSE(ok);
}

TEST(no_words_gives_an_empty_string) {
    char out[1];
    EXPECT_TRUE(initials("   ", out, sizeof out));
    EXPECT_STR_EQ(out, "");
}

TEST(zero_capacity_writes_nothing) {
    char out[1] = {'#'};
    EXPECT_FALSE(initials("", out, 0));
    EXPECT_EQ(out[0], '#');
}
