#include <stddef.h>

TEST(copies_a_name_into_a_fresh_buffer) {
    char header[16] = {0};
    copy_display_name(header, "ada", sizeof header);
    EXPECT_STR_EQ(header, "ada");
}

TEST(cuts_a_long_name_to_fit) {
    char tiny[6] = {0};
    copy_display_name(tiny, "alexandra", sizeof tiny);
    EXPECT_STR_EQ(tiny, "alexa");
}

TEST(a_shorter_name_replaces_a_longer_one) {
    char header[16] = "administrator";
    copy_display_name(header, "bob", sizeof header);
    EXPECT_STR_EQ(header, "bob");
}

TEST(an_empty_name_clears_the_buffer) {
    char header[8] = "guest";
    copy_display_name(header, "", sizeof header);
    EXPECT_STR_EQ(header, "");
}

TEST(a_one_byte_buffer_holds_only_the_terminator) {
    char header[4] = "abc";
    copy_display_name(header, "zed", 1);
    EXPECT_STR_EQ(header, "");
}

TEST(a_name_that_exactly_fits) {
    char header[5] = {0};
    copy_display_name(header, "lars", sizeof header);
    EXPECT_STR_EQ(header, "lars");
}
