#include <stdbool.h>
#include <stdint.h>

TEST(bit_is_set_reads_one_bit) {
    EXPECT_TRUE(bit_is_set(0x10u, 4));
    EXPECT_FALSE(bit_is_set(0x10u, 3));
    EXPECT_FALSE(bit_is_set(0xFFFFFFEFu, 4));
    EXPECT_TRUE(bit_is_set(0x80000000u, 31));
}

TEST(bit_31_works_without_signed_overflow) {
    EXPECT_TRUE(bit_is_set(0xFFFFFFFFu, 31));
    EXPECT_FALSE(bit_is_set(0x7FFFFFFFu, 31));
    EXPECT_TRUE(bit_is_set(0x80000001u, 0));
}

TEST(count_set_bits_counts_ones) {
    EXPECT_EQ(count_set_bits(0u), 0);
    EXPECT_EQ(count_set_bits(1u), 1);
    EXPECT_EQ(count_set_bits(0xF0F0u), 8);
    EXPECT_EQ(count_set_bits(0x80000001u), 2);
    EXPECT_EQ(count_set_bits(0xFFFFFFFFu), 32);
}

TEST(powers_of_two_have_exactly_one_bit) {
    EXPECT_TRUE(is_power_of_two(1u));
    EXPECT_TRUE(is_power_of_two(64u));
    EXPECT_TRUE(is_power_of_two(0x80000000u));
    EXPECT_FALSE(is_power_of_two(0u));
    EXPECT_FALSE(is_power_of_two(6u));
    EXPECT_FALSE(is_power_of_two(0xFFFFFFFFu));
}

TEST(reverse_bits_mirrors_the_word) {
    EXPECT_UEQ(reverse_bits(0x00000001u), 0x80000000u);
    EXPECT_UEQ(reverse_bits(0x80000000u), 0x00000001u);
    EXPECT_UEQ(reverse_bits(0x0000000Fu), 0xF0000000u);
    EXPECT_UEQ(reverse_bits(0x12345678u), 0x1E6A2C48u);
    EXPECT_UEQ(reverse_bits(0u), 0u);
    EXPECT_UEQ(reverse_bits(0xFFFFFFFFu), 0xFFFFFFFFu);
}
