#include <stddef.h>

TEST(signature_is_const_correct) {
    int ok = _Generic(&file_extension, const char *(*)(const char *): 1, default: 0);
    EXPECT_TRUE(ok);
}

TEST(returns_a_pointer_into_the_path) {
    const char *path = "report.pdf";
    const char *ext = file_extension(path);
    EXPECT_PTR_EQ(ext, path + 7);
    EXPECT_STR_EQ(ext, "pdf");
}

TEST(uses_the_last_dot) {
    EXPECT_STR_EQ(file_extension("backup.tar.gz"), "gz");
}

TEST(no_dot_returns_null) {
    EXPECT_NULL(file_extension("Makefile"));
    EXPECT_NULL(file_extension(""));
}

TEST(trailing_dot_gives_an_empty_extension) {
    const char *path = "draft.";
    const char *ext = file_extension(path);
    EXPECT_PTR_EQ(ext, path + 6);
    EXPECT_STR_EQ(ext, "");
}

TEST(works_on_a_read_only_exact_size_buffer) {
    const char path[5] = {'a', '.', 'c', 'x', '\0'};
    EXPECT_STR_EQ(file_extension(path), "cx");
}
