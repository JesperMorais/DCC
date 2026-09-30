#include <stdlib.h>
#include <string.h>

TEST(copies_the_contents) {
    char *copy = my_strdup("imu-0");
    EXPECT_STR_EQ(copy, "imu-0");
    free(copy);
}

TEST(returns_a_new_pointer_not_the_original) {
    const char *original = "barometer";
    char *copy = my_strdup(original);
    EXPECT_NOT_NULL(copy);
    bool same = copy == original;
    free(copy);
    EXPECT_FALSE(same);
}

TEST(copy_is_independent_of_the_original) {
    char buffer[16] = "gps-1";
    char *copy = my_strdup(buffer);
    buffer[4] = '9';               // the driver reuses its buffer
    EXPECT_STR_EQ(buffer, "gps-9");
    EXPECT_STR_EQ(copy, "gps-1");
    free(copy);
}

TEST(empty_string_gives_an_empty_copy) {
    char *copy = my_strdup("");
    EXPECT_NOT_NULL(copy);
    EXPECT_STR_EQ(copy, "");
    free(copy);
}

TEST(null_gives_null) {
    EXPECT_NULL(my_strdup(NULL));
}

TEST(copies_a_long_string_exactly) {
    char text[301];
    for (size_t i = 0; i < 300; i++) text[i] = (char)('a' + i % 26);
    text[300] = '\0';
    char *copy = my_strdup(text);
    EXPECT_EQ(strlen(copy), 300);
    EXPECT_STR_EQ(copy, text);
    free(copy);
}
