#include <stddef.h>

TEST(reverses_an_odd_length_playlist) {
    int playlist[] = {101, 202, 303, 404, 505};
    int backwards[5] = {0};
    int expected[] = {505, 404, 303, 202, 101};
    reverse_into(playlist, 5, backwards);
    EXPECT_INT_ARRAY_EQ(backwards, expected, 5);
}

TEST(reverses_an_even_length_playlist) {
    int playlist[] = {1, 2, 3, 4};
    int backwards[4] = {0};
    int expected[] = {4, 3, 2, 1};
    reverse_into(playlist, 4, backwards);
    EXPECT_INT_ARRAY_EQ(backwards, expected, 4);
}

TEST(original_playlist_is_unchanged) {
    int playlist[] = {7, 8, 9};
    int backwards[3] = {0};
    int original[] = {7, 8, 9};
    reverse_into(playlist, 3, backwards);
    EXPECT_INT_ARRAY_EQ(playlist, original, 3);
}

TEST(a_single_song) {
    int playlist[] = {42};
    int backwards[1] = {0};
    reverse_into(playlist, 1, backwards);
    EXPECT_EQ(backwards[0], 42);
}

TEST(empty_playlist_writes_nothing) {
    int playlist[] = {1};
    int backwards[1] = {-99};
    reverse_into(playlist, 0, backwards);
    EXPECT_EQ(backwards[0], -99);
}
