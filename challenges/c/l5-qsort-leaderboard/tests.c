#include <limits.h>
#include <stddef.h>
#include <string.h>

TEST(highest_score_first) {
    Player board[] = {{"ada", 50}, {"bob", 90}, {"cy", 70}};
    sort_leaderboard(board, 3);
    EXPECT_STR_EQ(board[0].name, "bob");
    EXPECT_STR_EQ(board[1].name, "cy");
    EXPECT_STR_EQ(board[2].name, "ada");
    EXPECT_EQ(board[0].score, 90);
    EXPECT_EQ(board[2].score, 50);
}

TEST(ties_are_ordered_by_name) {
    Player board[] = {{"zed", 50}, {"bob", 90}, {"ada", 50}, {"mia", 50}};
    sort_leaderboard(board, 4);
    EXPECT_STR_EQ(board[0].name, "bob");
    EXPECT_STR_EQ(board[1].name, "ada");
    EXPECT_STR_EQ(board[2].name, "mia");
    EXPECT_STR_EQ(board[3].name, "zed");
}

TEST(negative_scores_sort_below_zero) {
    Player board[] = {{"a", -5}, {"b", 0}, {"c", -100}, {"d", 3}};
    sort_leaderboard(board, 4);
    int scores[4];
    for (int i = 0; i < 4; i++) scores[i] = board[i].score;
    int expected[] = {3, 0, -5, -100};
    EXPECT_INT_ARRAY_EQ(scores, expected, 4);
}

TEST(extreme_scores_do_not_overflow) {
    Player board[] = {{"low", INT_MIN}, {"high", INT_MAX}, {"mid", 0}, {"low2", INT_MIN}};
    sort_leaderboard(board, 4);
    EXPECT_STR_EQ(board[0].name, "high");
    EXPECT_STR_EQ(board[1].name, "mid");
    EXPECT_STR_EQ(board[2].name, "low");
    EXPECT_STR_EQ(board[3].name, "low2");
}

TEST(empty_and_single_boards) {
    Player one[] = {{"solo", 1}};
    sort_leaderboard(one, 0);
    sort_leaderboard(one, 1);
    EXPECT_STR_EQ(one[0].name, "solo");
    EXPECT_EQ(one[0].score, 1);
}
