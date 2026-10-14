#include <string.h>

TEST(strips_a_windows_line_ending) {
    char line[] = "disk full\r\n";
    EXPECT_EQ(chomp_line(line), 9);
    EXPECT_STR_EQ(line, "disk full");
}

TEST(strips_trailing_spaces_and_tabs_but_keeps_leading_ones) {
    char line[] = "  indented  \t";
    EXPECT_EQ(chomp_line(line), 10);
    EXPECT_STR_EQ(line, "  indented");
}

TEST(keeps_whitespace_in_the_middle) {
    char line[] = "user logged in\n";
    EXPECT_EQ(chomp_line(line), 14);
    EXPECT_STR_EQ(line, "user logged in");
}

TEST(a_line_of_only_whitespace_becomes_empty) {
    char line[] = " \r\n";
    EXPECT_EQ(chomp_line(line), 0);
    EXPECT_STR_EQ(line, "");
}

TEST(a_clean_line_is_left_alone) {
    char line[] = "boot ok";
    EXPECT_EQ(chomp_line(line), 7);
    EXPECT_STR_EQ(line, "boot ok");
}

TEST(empty_string_stays_empty) {
    char line[] = "";
    EXPECT_EQ(chomp_line(line), 0);
    EXPECT_STR_EQ(line, "");
}
