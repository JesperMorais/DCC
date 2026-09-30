TEST(four_rows_need_ten_cans) {
    EXPECT_EQ(sum_one_to_n(4), 10);
}

TEST(a_single_row_is_one_can) {
    EXPECT_EQ(sum_one_to_n(1), 1);
}

TEST(a_hundred_rows) {
    EXPECT_EQ(sum_one_to_n(100), 5050);
}

TEST(zero_rows_need_no_cans) {
    EXPECT_EQ(sum_one_to_n(0), 0);
}

TEST(negative_rows_need_no_cans) {
    EXPECT_EQ(sum_one_to_n(-5), 0);
}
