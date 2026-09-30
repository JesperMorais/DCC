TEST(capitalizes_each_word) {
    char buf[] = "hey jude";
    capitalize_words(buf);
    EXPECT_STR_EQ(buf, "Hey Jude");
}

TEST(lowercases_the_rest_of_each_word) {
    char buf[] = "bOHEMIAN RHAPSODY";
    capitalize_words(buf);
    EXPECT_STR_EQ(buf, "Bohemian Rhapsody");
}

TEST(keeps_runs_of_whitespace_untouched) {
    char buf[] = "  let\tit   be ";
    capitalize_words(buf);
    EXPECT_STR_EQ(buf, "  Let\tIt   Be ");
}

TEST(non_letters_stay_as_they_are) {
    char buf[] = "99 red balloons!";
    capitalize_words(buf);
    EXPECT_STR_EQ(buf, "99 Red Balloons!");
}

TEST(empty_string_stays_empty) {
    char buf[] = "";
    capitalize_words(buf);
    EXPECT_STR_EQ(buf, "");
}

TEST(single_letter_in_an_exact_size_buffer) {
    char buf[2] = {'x', '\0'};
    capitalize_words(buf);
    EXPECT_STR_EQ(buf, "X");
}
