TEST(digit_pairs) {
    EXPECT_EQ(hex_pair_value("00"), 0);
    EXPECT_EQ(hex_pair_value("33"), 51);
    EXPECT_EQ(hex_pair_value("99"), 153);
}

TEST(uppercase_letters_are_ten_to_fifteen) {
    EXPECT_EQ(hex_pair_value("FF"), 255);
    EXPECT_EQ(hex_pair_value("0A"), 10);
    EXPECT_EQ(hex_pair_value("B4"), 180);
}

TEST(lowercase_letters_work_too) {
    EXPECT_EQ(hex_pair_value("c0"), 192);
    EXPECT_EQ(hex_pair_value("3e"), 62);
}

TEST(only_the_first_two_characters_count) {
    EXPECT_EQ(hex_pair_value("6699"), 102);
}

TEST(rejects_a_bad_first_character) {
    EXPECT_EQ(hex_pair_value("G7"), -1);
}

TEST(rejects_a_bad_second_character) {
    EXPECT_EQ(hex_pair_value("7z"), -1);
    EXPECT_EQ(hex_pair_value("1 "), -1);
}
