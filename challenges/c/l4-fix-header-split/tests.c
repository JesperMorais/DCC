#include <stdbool.h>
#include <stddef.h>

TEST(splits_a_simple_header) {
    char line[] = "Host: example.com";
    char *name = NULL, *value = NULL;
    EXPECT_TRUE(split_header(line, &name, &value));
    EXPECT_STR_EQ(name, "Host");
    EXPECT_STR_EQ(value, "example.com");
}

TEST(no_space_after_the_colon) {
    char line[] = "Content-Length:42";
    char *name = NULL, *value = NULL;
    EXPECT_TRUE(split_header(line, &name, &value));
    EXPECT_STR_EQ(name, "Content-Length");
    EXPECT_STR_EQ(value, "42");
}

TEST(value_may_contain_colons) {
    char line[] = "Host: example.com:8080";
    char *name = NULL, *value = NULL;
    EXPECT_TRUE(split_header(line, &name, &value));
    EXPECT_STR_EQ(name, "Host");
    EXPECT_STR_EQ(value, "example.com:8080");
}

TEST(value_with_a_url_and_a_time) {
    char referer[] = "Referer:   https://shop.example/cart";
    char date[] = "Date: Wed, 14 Oct 2026 08:30:00 GMT";
    char *name = NULL, *value = NULL;
    EXPECT_TRUE(split_header(referer, &name, &value));
    EXPECT_STR_EQ(value, "https://shop.example/cart");
    EXPECT_TRUE(split_header(date, &name, &value));
    EXPECT_STR_EQ(name, "Date");
    EXPECT_STR_EQ(value, "Wed, 14 Oct 2026 08:30:00 GMT");
}

TEST(empty_value_is_allowed) {
    char line[] = "X-Debug:";
    char *name = NULL, *value = NULL;
    EXPECT_TRUE(split_header(line, &name, &value));
    EXPECT_STR_EQ(name, "X-Debug");
    EXPECT_STR_EQ(value, "");
}

TEST(rejects_a_line_without_a_colon) {
    char line[] = "no colon here";
    char *name = NULL, *value = NULL;
    EXPECT_FALSE(split_header(line, &name, &value));
    EXPECT_STR_EQ(line, "no colon here");
    EXPECT_NULL(name);
    EXPECT_NULL(value);
}

TEST(rejects_an_empty_name) {
    char line[] = ": value";
    char *name = NULL, *value = NULL;
    EXPECT_FALSE(split_header(line, &name, &value));
    EXPECT_STR_EQ(line, ": value");
    EXPECT_NULL(name);
}
