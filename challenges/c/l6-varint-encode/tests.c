#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

TEST(small_values_take_one_byte) {
    uint8_t buf[10];
    EXPECT_EQ(varint_encode(0, buf, sizeof buf), 1);
    EXPECT_UEQ(buf[0], 0x00);
    EXPECT_EQ(varint_encode(1, buf, sizeof buf), 1);
    EXPECT_UEQ(buf[0], 0x01);
    EXPECT_EQ(varint_encode(127, buf, sizeof buf), 1);
    EXPECT_UEQ(buf[0], 0x7F);
}

TEST(values_from_128_need_a_continuation_byte) {
    uint8_t buf[10];
    EXPECT_EQ(varint_encode(128, buf, sizeof buf), 2);
    EXPECT_UEQ(buf[0], 0x80);
    EXPECT_UEQ(buf[1], 0x01);
    EXPECT_EQ(varint_encode(300, buf, sizeof buf), 2);
    EXPECT_UEQ(buf[0], 0xAC);
    EXPECT_UEQ(buf[1], 0x02);
}

TEST(three_byte_boundary) {
    uint8_t buf[10];
    EXPECT_EQ(varint_encode(16383, buf, sizeof buf), 2);
    EXPECT_UEQ(buf[0], 0xFF);
    EXPECT_UEQ(buf[1], 0x7F);
    EXPECT_EQ(varint_encode(16384, buf, sizeof buf), 3);
    EXPECT_UEQ(buf[0], 0x80);
    EXPECT_UEQ(buf[1], 0x80);
    EXPECT_UEQ(buf[2], 0x01);
}

TEST(largest_value_takes_ten_bytes) {
    uint8_t buf[10];
    EXPECT_EQ(varint_encode(UINT64_MAX, buf, sizeof buf), 10);
    for (int i = 0; i < 9; i++) EXPECT_UEQ(buf[i], 0xFF);
    EXPECT_UEQ(buf[9], 0x01);
}

TEST(fits_exactly_in_a_buffer_of_the_right_size) {
    uint8_t *buf = malloc(3);   // ASan watches every byte past these 3
    size_t n = varint_encode(1000000, buf, 3);   // 0x0F4240 → C0 84 3D
    uint8_t got[3];
    memcpy(got, buf, 3);
    free(buf);
    EXPECT_EQ(n, 3);
    EXPECT_UEQ(got[0], 0xC0);
    EXPECT_UEQ(got[1], 0x84);
    EXPECT_UEQ(got[2], 0x3D);
}

TEST(too_small_buffer_returns_0_and_writes_nothing) {
    uint8_t *buf = malloc(2);
    buf[0] = 0xEE;
    buf[1] = 0xEE;
    size_t n = varint_encode(1000000, buf, 2);   // needs 3 bytes
    uint8_t b0 = buf[0], b1 = buf[1];
    free(buf);
    EXPECT_EQ(n, 0);
    EXPECT_UEQ(b0, 0xEE);
    EXPECT_UEQ(b1, 0xEE);
    uint8_t one[1] = {0xEE};
    EXPECT_EQ(varint_encode(128, one, 1), 0);
    EXPECT_UEQ(one[0], 0xEE);
    EXPECT_EQ(varint_encode(5, one, 0), 0);
    EXPECT_UEQ(one[0], 0xEE);
}
