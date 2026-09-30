TEST(counts_letters_in_a_word) {
    int counts[26] = {0};
    letter_counts("banana", counts);
    EXPECT_EQ(counts['a' - 'a'], 3);
    EXPECT_EQ(counts['n' - 'a'], 2);
    EXPECT_EQ(counts['b' - 'a'], 1);
    EXPECT_EQ(counts['z' - 'a'], 0);
}

TEST(upper_and_lowercase_are_the_same_letter) {
    int counts[26] = {0};
    letter_counts("EeEe", counts);
    EXPECT_EQ(counts['e' - 'a'], 4);
}

TEST(first_and_last_letters_of_the_alphabet) {
    int counts[26] = {0};
    letter_counts("Aaz Z", counts);
    EXPECT_EQ(counts[0], 2);
    EXPECT_EQ(counts[25], 2);
}

TEST(ignores_digits_spaces_and_punctuation) {
    int counts[26] = {0};
    letter_counts("@[`{ 0-9 ?!", counts);
    int expected[26] = {0};
    EXPECT_INT_ARRAY_EQ(counts, expected, 26);
}

TEST(overwrites_whatever_was_in_the_array) {
    int counts[26];
    for (int k = 0; k < 26; k++) counts[k] = 99;
    letter_counts("Hello!", counts);
    int expected[26] = {0};
    expected['e' - 'a'] = 1;
    expected['h' - 'a'] = 1;
    expected['l' - 'a'] = 2;
    expected['o' - 'a'] = 1;
    EXPECT_INT_ARRAY_EQ(counts, expected, 26);
}

TEST(empty_string_gives_all_zeros) {
    int counts[26];
    for (int k = 0; k < 26; k++) counts[k] = -1;
    letter_counts("", counts);
    int expected[26] = {0};
    EXPECT_INT_ARRAY_EQ(counts, expected, 26);
}
