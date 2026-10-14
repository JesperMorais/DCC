#include <stdlib.h>
#include <string.h>

TEST(adds_a_slash_between_the_parts) {
    char *p = path_join("logs", "app.txt");
    EXPECT_STR_EQ(p, "logs/app.txt");
    free(p);
}

TEST(does_not_double_an_existing_slash) {
    char *p = path_join("/var/log/", "syslog");
    EXPECT_STR_EQ(p, "/var/log/syslog");
    free(p);
}

TEST(root_directory) {
    char *p = path_join("/", "etc");
    EXPECT_STR_EQ(p, "/etc");
    free(p);
}

TEST(empty_dir_gives_just_the_name) {
    char *p = path_join("", "notes.md");
    EXPECT_STR_EQ(p, "notes.md");
    free(p);
}

TEST(empty_name_after_a_dir) {
    char *p = path_join("cache", "");
    EXPECT_STR_EQ(p, "cache/");
    free(p);
}

TEST(joins_long_paths_exactly) {
    char dir[201];
    memset(dir, 'd', 200);
    dir[200] = '\0';
    char *p = path_join(dir, "file.bin");
    EXPECT_EQ(strlen(p), 209);
    EXPECT_EQ(p[200], '/');
    EXPECT_STR_EQ(p + 201, "file.bin");
    free(p);
}
