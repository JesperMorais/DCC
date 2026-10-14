#include <stdbool.h>
#include <string.h>

TEST(appends_pieces_in_order) {
    StrBuf sb;
    strbuf_init(&sb);
    EXPECT_TRUE(strbuf_append(&sb, "{\"id\":"));
    EXPECT_TRUE(strbuf_append(&sb, "42}"));
    EXPECT_STR_EQ(sb.data, "{\"id\":42}");
    EXPECT_EQ(sb.len, 9);
    strbuf_free(&sb);
}

TEST(appending_an_empty_string_to_a_new_builder) {
    StrBuf sb;
    strbuf_init(&sb);
    EXPECT_TRUE(strbuf_append(&sb, ""));
    EXPECT_STR_EQ(sb.data, "");
    EXPECT_EQ(sb.len, 0);
    strbuf_free(&sb);
}

TEST(many_small_appends_grow_the_buffer) {
    StrBuf sb;
    strbuf_init(&sb);
    for (int i = 0; i < 100; i++) EXPECT_TRUE(strbuf_append(&sb, "ab"));
    EXPECT_EQ(sb.len, 200);
    EXPECT_EQ(strlen(sb.data), 200);
    EXPECT_EQ(sb.data[199], 'b');
    strbuf_free(&sb);
}

TEST(one_long_append_to_an_empty_builder) {
    StrBuf sb;
    strbuf_init(&sb);
    EXPECT_TRUE(strbuf_append(&sb, "serial=SN-2026-0042-ABCDEFGH-REV-C"));
    EXPECT_EQ(sb.len, 34);
    EXPECT_STR_EQ(sb.data, "serial=SN-2026-0042-ABCDEFGH-REV-C");
    strbuf_free(&sb);
}

TEST(a_long_append_after_some_content) {
    StrBuf sb;
    strbuf_init(&sb);
    strbuf_append(&sb, "log:");
    char big[301];
    memset(big, 'x', 300);
    big[300] = '\0';
    EXPECT_TRUE(strbuf_append(&sb, big));
    EXPECT_EQ(sb.len, 304);
    EXPECT_EQ(strncmp(sb.data, "log:xxx", 7), 0);
    EXPECT_EQ(sb.data[304], '\0');
    strbuf_free(&sb);
}

TEST(free_leaves_an_empty_reusable_builder) {
    StrBuf sb;
    strbuf_init(&sb);
    strbuf_append(&sb, "temporary");
    strbuf_free(&sb);
    EXPECT_NULL(sb.data);
    EXPECT_EQ(sb.len, 0);
    EXPECT_TRUE(strbuf_append(&sb, "again"));
    EXPECT_STR_EQ(sb.data, "again");
    strbuf_free(&sb);
}
