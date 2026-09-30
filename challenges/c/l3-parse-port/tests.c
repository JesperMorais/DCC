#include <stdbool.h>

TEST(parses_a_normal_port) {
    unsigned port = 0;
    EXPECT_TRUE(parse_port("8080", &port));
    EXPECT_UEQ(port, 8080);
}

TEST(accepts_both_ends_of_the_range) {
    unsigned port = 1;
    EXPECT_TRUE(parse_port("0", &port));
    EXPECT_UEQ(port, 0);
    EXPECT_TRUE(parse_port("65535", &port));
    EXPECT_UEQ(port, 65535);
}

TEST(leading_zeros_are_fine) {
    unsigned port = 0;
    EXPECT_TRUE(parse_port("080", &port));
    EXPECT_UEQ(port, 80);
}

TEST(rejects_non_digits_and_leaves_out_alone) {
    unsigned port = 1234;
    EXPECT_FALSE(parse_port("80a", &port));
    EXPECT_FALSE(parse_port("-1", &port));
    EXPECT_FALSE(parse_port(" 80", &port));
    EXPECT_FALSE(parse_port("80 ", &port));
    EXPECT_UEQ(port, 1234);
}

TEST(rejects_an_empty_string) {
    unsigned port = 1234;
    EXPECT_FALSE(parse_port("", &port));
    EXPECT_UEQ(port, 1234);
}

TEST(rejects_values_that_are_too_big_even_if_they_wrap) {
    unsigned port = 1234;
    EXPECT_FALSE(parse_port("65536", &port));
    // 4294967376 is 2^32 + 80: an unchecked unsigned wraps it to 80.
    EXPECT_FALSE(parse_port("4294967376", &port));
    EXPECT_FALSE(parse_port("99999999999999999999", &port));
    EXPECT_UEQ(port, 1234);
}

TEST(single_digit_in_an_exact_size_buffer) {
    char buf[2] = {'7', '\0'};
    unsigned port = 0;
    EXPECT_TRUE(parse_port(buf, &port));
    EXPECT_UEQ(port, 7);
}
