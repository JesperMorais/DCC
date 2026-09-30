#include <limits.h>
#include <stdbool.h>
#include <stddef.h>

TEST(parses_a_number_and_points_past_it) {
    const char *s = "42ms";
    const char *end = NULL;
    int v = 0;
    EXPECT_TRUE(parse_int_prefix(s, &v, &end));
    EXPECT_EQ(v, 42);
    EXPECT_PTR_EQ(end, s + 2);
    EXPECT_STR_EQ(end, "ms");
}

TEST(signs_and_leading_zeros) {
    const char *s1 = "-17", *s2 = "+8", *s3 = "007";
    const char *end = NULL;
    int v = 0;
    EXPECT_TRUE(parse_int_prefix(s1, &v, &end));
    EXPECT_EQ(v, -17);
    EXPECT_PTR_EQ(end, s1 + 3);
    EXPECT_TRUE(parse_int_prefix(s2, &v, &end));
    EXPECT_EQ(v, 8);
    EXPECT_PTR_EQ(end, s2 + 2);
    EXPECT_TRUE(parse_int_prefix(s3, &v, &end));
    EXPECT_EQ(v, 7);
    EXPECT_PTR_EQ(end, s3 + 3);
}

TEST(walks_a_gcode_line_with_the_end_pointer) {
    const char *line = "X120Y-45";
    const char *rest = NULL;
    int x = 0, y = 0;
    EXPECT_TRUE(parse_int_prefix(line + 1, &x, &rest));
    EXPECT_EQ(x, 120);
    EXPECT_STR_EQ(rest, "Y-45");
    EXPECT_TRUE(parse_int_prefix(rest + 1, &y, &rest));
    EXPECT_EQ(y, -45);
    EXPECT_STR_EQ(rest, "");
}

TEST(no_digits_is_an_error_that_consumes_nothing) {
    const char *inputs[] = {"F", "", "-", "+", "--5", " 5", "-x"};
    for (size_t i = 0; i < sizeof inputs / sizeof inputs[0]; i++) {
        const char *end = NULL;
        int v = 555;
        EXPECT_FALSE(parse_int_prefix(inputs[i], &v, &end));
        EXPECT_PTR_EQ(end, inputs[i]);
        EXPECT_EQ(v, 555);
    }
}

TEST(accepts_the_exact_int_limits) {
    const char *end = NULL;
    int v = 0;
    EXPECT_TRUE(parse_int_prefix("2147483647", &v, &end));
    EXPECT_EQ(v, INT_MAX);
    EXPECT_TRUE(parse_int_prefix("-2147483648", &v, &end));
    EXPECT_EQ(v, INT_MIN);
    EXPECT_STR_EQ(end, "");
}

TEST(out_of_range_is_an_error_without_overflowing) {
    const char *inputs[] = {"2147483648", "-2147483649", "+99999999999", "18446744073709551616"};
    for (size_t i = 0; i < sizeof inputs / sizeof inputs[0]; i++) {
        const char *end = NULL;
        int v = 555;
        EXPECT_FALSE(parse_int_prefix(inputs[i], &v, &end));
        EXPECT_PTR_EQ(end, inputs[i]);
        EXPECT_EQ(v, 555);
    }
}
