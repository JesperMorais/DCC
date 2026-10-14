#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

// Appends one record (type, big-endian 16-bit length, value) at buf + at.
static size_t put_record(char *buf, size_t at, uint8_t type, char fill, uint16_t length) {
    buf[at] = (char)type;
    buf[at + 1] = (char)(length >> 8);
    buf[at + 2] = (char)(length & 0xFF);
    memset(buf + at + 3, fill, length);
    return at + 3 + length;
}

TEST(reads_records_in_order_then_ends) {
    char buf[64];
    size_t size = put_record(buf, 0, 1, 'a', 4);
    size = put_record(buf, size, 7, 'b', 0);
    size = put_record(buf, size, 2, 'c', 10);
    TlvReader r;
    tlv_reader_init(&r, buf, size);
    Tlv t;
    EXPECT_EQ(tlv_next(&r, &t), TLV_OK);
    EXPECT_EQ(t.type, 1);
    EXPECT_EQ(t.length, 4);
    EXPECT_EQ(t.value[0], 'a');
    EXPECT_EQ(tlv_next(&r, &t), TLV_OK);
    EXPECT_EQ(t.type, 7);
    EXPECT_EQ(t.length, 0);
    EXPECT_EQ(tlv_next(&r, &t), TLV_OK);
    EXPECT_EQ(t.type, 2);
    EXPECT_EQ(t.length, 10);
    EXPECT_PTR_EQ(t.value, buf + 13);
    EXPECT_EQ(tlv_next(&r, &t), TLV_END);
}

TEST(type_bytes_above_127) {
    char buf[16];
    size_t size = put_record(buf, 0, 0xF0, 'x', 2);
    Tlv t;
    EXPECT_TRUE(tlv_find(buf, size, 0xF0, &t));
    EXPECT_EQ(t.length, 2);
}

TEST(reads_a_200_byte_value) {
    char buf[256];
    size_t size = put_record(buf, 0, 3, 'f', 200);
    TlvReader r;
    tlv_reader_init(&r, buf, size);
    Tlv t;
    EXPECT_EQ(tlv_next(&r, &t), TLV_OK);
    EXPECT_EQ(t.length, 200);
    EXPECT_EQ(t.value[199], 'f');
    EXPECT_EQ(tlv_next(&r, &t), TLV_END);
}

TEST(reads_values_longer_than_255_bytes) {
    static char buf[1024];
    size_t size = put_record(buf, 0, 4, 'g', 300);
    size = put_record(buf, size, 5, 'h', 500);
    TlvReader r;
    tlv_reader_init(&r, buf, size);
    Tlv t;
    EXPECT_EQ(tlv_next(&r, &t), TLV_OK);
    EXPECT_EQ(t.length, 300);
    EXPECT_EQ(tlv_next(&r, &t), TLV_OK);
    EXPECT_EQ(t.length, 500);
    EXPECT_EQ(t.value[499], 'h');
}

TEST(finds_a_record_after_a_long_one) {
    static char buf[1024];
    size_t size = put_record(buf, 0, 1, 'a', 3);
    size = put_record(buf, size, 2, 'b', 130);
    size = put_record(buf, size, 9, 'z', 1);
    Tlv t;
    EXPECT_TRUE(tlv_find(buf, size, 9, &t));
    EXPECT_EQ(t.value[0], 'z');
    EXPECT_FALSE(tlv_find(buf, size, 42, &t));
}

TEST(a_cut_off_header_or_value_is_truncated_and_does_not_advance) {
    char buf[32];
    size_t size = put_record(buf, 0, 1, 'a', 10);
    TlvReader r;
    Tlv t;
    tlv_reader_init(&r, buf, 2);
    EXPECT_EQ(tlv_next(&r, &t), TLV_TRUNCATED);
    EXPECT_EQ(r.pos, 0);
    tlv_reader_init(&r, buf, size - 1);
    EXPECT_EQ(tlv_next(&r, &t), TLV_TRUNCATED);
    EXPECT_EQ(r.pos, 0);
}

TEST(empty_input_ends_immediately) {
    TlvReader r;
    tlv_reader_init(&r, "", 0);
    Tlv t;
    EXPECT_EQ(tlv_next(&r, &t), TLV_END);
}
