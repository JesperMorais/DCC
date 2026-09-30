TEST(reverses_an_odd_length_word) {
    char buf[] = "READY";
    reverse_in_place(buf);
    EXPECT_STR_EQ(buf, "YDAER");
}

TEST(reverses_an_even_length_word) {
    char buf[] = "stop";
    reverse_in_place(buf);
    EXPECT_STR_EQ(buf, "pots");
}

TEST(reverses_spaces_and_punctuation_too) {
    char buf[] = "go, now!";
    reverse_in_place(buf);
    EXPECT_STR_EQ(buf, "!won ,og");
}

TEST(single_character_is_unchanged) {
    char buf[] = "a";
    reverse_in_place(buf);
    EXPECT_STR_EQ(buf, "a");
}

TEST(empty_string_is_unchanged) {
    char buf[] = "";
    reverse_in_place(buf);
    EXPECT_STR_EQ(buf, "");
}

TEST(two_characters_swap_and_the_terminator_stays_put) {
    char buf[3] = {'a', 'b', '\0'};
    reverse_in_place(buf);
    EXPECT_EQ(buf[0], 'b');
    EXPECT_EQ(buf[1], 'a');
    EXPECT_EQ(buf[2], '\0');
}
