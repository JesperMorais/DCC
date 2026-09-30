#include <stdlib.h>
#include <string.h>

TEST(joins_with_a_separator_between_parts) {
    const char *cols[] = {"time", "temp", "humidity"};
    char *row = join_strings(cols, 3, ", ");
    EXPECT_STR_EQ(row, "time, temp, humidity");
    free(row);
}

TEST(single_part_has_no_separator) {
    const char *cols[] = {"time"};
    char *row = join_strings(cols, 1, ", ");
    EXPECT_STR_EQ(row, "time");
    free(row);
}

TEST(zero_parts_gives_an_allocated_empty_string) {
    const char *cols[] = {"unused"};
    char *row = join_strings(cols, 0, ";");
    EXPECT_NOT_NULL(row);
    EXPECT_STR_EQ(row, "");
    free(row);
}

TEST(empty_parts_still_get_separators) {
    const char *cols[] = {"a", "", "b", ""};
    char *row = join_strings(cols, 4, ";");
    EXPECT_STR_EQ(row, "a;;b;");
    free(row);
}

TEST(empty_separator_just_concatenates) {
    const char *cols[] = {"sen", "sor", "42"};
    char *row = join_strings(cols, 3, "");
    EXPECT_STR_EQ(row, "sensor42");
    free(row);
}

TEST(result_is_exactly_as_long_as_it_should_be) {
    const char *cols[] = {"x", "yy", "zzz"};
    char *row = join_strings(cols, 3, " | ");
    EXPECT_NOT_NULL(row);
    size_t len = strlen(row);
    free(row);
    EXPECT_EQ(len, 12);   // 1 + 2 + 3 letters, 2 separators of 3
}
