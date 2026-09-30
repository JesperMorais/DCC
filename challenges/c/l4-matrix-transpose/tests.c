#include <stddef.h>
#include <stdlib.h>

TEST(flips_a_wide_table) {
    const int sales[2][3] = {{10, 20, 30}, {40, 50, 60}};
    int by_day[3][2] = {{0}};
    transpose(2, 3, sales, by_day);
    int expected[6] = {10, 40, 20, 50, 30, 60};
    EXPECT_INT_ARRAY_EQ(&by_day[0][0], expected, 6);
}

TEST(flips_a_tall_table) {
    const int in[3][2] = {{1, 2}, {3, 4}, {5, 6}};
    int out[2][3] = {{0}};
    transpose(3, 2, in, out);
    int expected[6] = {1, 3, 5, 2, 4, 6};
    EXPECT_INT_ARRAY_EQ(&out[0][0], expected, 6);
}

TEST(square_table) {
    const int in[2][2] = {{1, 2}, {3, 4}};
    int out[2][2] = {{0}};
    transpose(2, 2, in, out);
    EXPECT_EQ(out[0][1], 3);
    EXPECT_EQ(out[1][0], 2);
    EXPECT_EQ(out[0][0], 1);
    EXPECT_EQ(out[1][1], 4);
}

TEST(single_row_becomes_a_single_column) {
    const int in[1][4] = {{7, 8, 9, 10}};
    int out[4][1] = {{0}};
    transpose(1, 4, in, out);
    int expected[4] = {7, 8, 9, 10};
    EXPECT_INT_ARRAY_EQ(&out[0][0], expected, 4);
}

TEST(stays_inside_exact_size_heap_tables) {
    const size_t rows = 2, cols = 5;
    int (*in)[5] = malloc(rows * sizeof *in);
    int (*out)[2] = malloc(cols * sizeof *out);
    for (size_t r = 0; r < rows; r++)
        for (size_t c = 0; c < cols; c++) in[r][c] = (int)(r * 10 + c);
    transpose(rows, cols, (const int (*)[5])in, out);
    int got[10];
    for (size_t c = 0; c < cols; c++)
        for (size_t r = 0; r < rows; r++) got[c * rows + r] = out[c][r];
    free(in);
    free(out);
    int expected[10] = {0, 10, 1, 11, 2, 12, 3, 13, 4, 14};
    EXPECT_INT_ARRAY_EQ(got, expected, 10);
}
