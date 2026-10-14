TEST(counts_words_separated_by_single_spaces) {
    EXPECT_EQ(caption_word_count("I know"), 2);
    EXPECT_EQ(caption_word_count("we need to talk"), 4);
}

TEST(extra_whitespace_around_words_is_ignored) {
    EXPECT_EQ(caption_word_count("  where are you  "), 3);
}

TEST(tabs_and_newlines_separate_words) {
    EXPECT_EQ(caption_word_count("wait\tfor\nme"), 3);
}

TEST(a_single_word) {
    EXPECT_EQ(caption_word_count("Run!"), 1);
}

TEST(empty_caption_has_no_words) {
    EXPECT_EQ(caption_word_count(""), 0);
}

TEST(only_whitespace_has_no_words) {
    EXPECT_EQ(caption_word_count(" \t \n"), 0);
}
