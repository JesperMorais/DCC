TEST(equal_apart_from_case_is_zero) {
    EXPECT_EQ(compare_ignore_case("Alice", "aLiCe"), 0);
    EXPECT_EQ(compare_ignore_case("bob", "bob"), 0);
}

TEST(earlier_name_is_negative) {
    EXPECT_TRUE(compare_ignore_case("bob", "Carol") < 0);
    EXPECT_TRUE(compare_ignore_case("Bob", "carol") < 0);
}

TEST(later_name_is_positive) {
    EXPECT_TRUE(compare_ignore_case("zoe", "Zack") > 0);
}

TEST(a_prefix_sorts_first) {
    EXPECT_TRUE(compare_ignore_case("ann", "Anna") < 0);
    EXPECT_TRUE(compare_ignore_case("ANNA", "ann") > 0);
}

TEST(empty_strings) {
    EXPECT_EQ(compare_ignore_case("", ""), 0);
    EXPECT_TRUE(compare_ignore_case("", "a") < 0);
    EXPECT_TRUE(compare_ignore_case("a", "") > 0);
}

TEST(stops_at_the_terminator_of_both_strings) {
    char a[3] = {'J', 'o', '\0'};
    char b[3] = {'j', 'O', '\0'};
    EXPECT_EQ(compare_ignore_case(a, b), 0);
    char c[2] = {'x', '\0'};
    char d[3] = {'x', 'y', '\0'};
    EXPECT_TRUE(compare_ignore_case(c, d) < 0);
}
