#include <stddef.h>

TEST(counts_a_single_word) {
    EXPECT_UEQ(my_strlen("hello"), 5);
}

TEST(spaces_and_punctuation_count) {
    EXPECT_UEQ(my_strlen("see you at 8!"), 13);
}

TEST(empty_string_is_zero) {
    EXPECT_UEQ(my_strlen(""), 0);
}

TEST(single_character) {
    EXPECT_UEQ(my_strlen("x"), 1);
}

TEST(stops_at_the_first_terminator) {
    char buf[] = "ok\0hidden";
    EXPECT_UEQ(my_strlen(buf), 2);
}

TEST(does_not_read_past_the_end_of_the_buffer) {
    char exact[4] = {'a', 'b', 'c', '\0'};
    EXPECT_UEQ(my_strlen(exact), 3);
}
