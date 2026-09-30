#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

TEST(new_map_is_empty) {
    StrMap *m = strmap_new();
    EXPECT_NOT_NULL(m);
    int v = 5;
    bool found = strmap_get(m, "baud", &v);
    size_t size = strmap_size(m);
    strmap_free(m);
    EXPECT_FALSE(found);
    EXPECT_EQ(v, 5);
    EXPECT_EQ(size, 0);
}

TEST(put_then_get) {
    StrMap *m = strmap_new();
    EXPECT_NOT_NULL(m);
    bool ok = strmap_put(m, "baud", 9600) && strmap_put(m, "stop_bits", 1) && strmap_put(m, "retries", -3);
    int baud = 0, stop = 0, retries = 0, missing = 42;
    bool got = strmap_get(m, "baud", &baud) && strmap_get(m, "stop_bits", &stop) && strmap_get(m, "retries", &retries);
    bool found_missing = strmap_get(m, "parity", &missing);
    size_t size = strmap_size(m);
    strmap_free(m);
    EXPECT_TRUE(ok);
    EXPECT_TRUE(got);
    EXPECT_EQ(baud, 9600);
    EXPECT_EQ(stop, 1);
    EXPECT_EQ(retries, -3);
    EXPECT_FALSE(found_missing);
    EXPECT_EQ(missing, 42);
    EXPECT_EQ(size, 3);
}

TEST(put_overwrites_an_existing_key) {
    StrMap *m = strmap_new();
    EXPECT_NOT_NULL(m);
    strmap_put(m, "baud", 9600);
    strmap_put(m, "baud", 115200);
    int v = 0;
    bool found = strmap_get(m, "baud", &v);
    size_t size = strmap_size(m);
    strmap_free(m);
    EXPECT_TRUE(found);
    EXPECT_EQ(v, 115200);
    EXPECT_EQ(size, 1);
}

TEST(map_keeps_its_own_copy_of_the_key) {
    StrMap *m = strmap_new();
    EXPECT_NOT_NULL(m);
    char buf[16];
    strcpy(buf, "alpha");
    strmap_put(m, buf, 1);
    strcpy(buf, "beta");     // the caller reuses its buffer
    strmap_put(m, buf, 2);
    int a = 0, b = 0;
    bool got = strmap_get(m, "alpha", &a) && strmap_get(m, "beta", &b);
    size_t size = strmap_size(m);
    strmap_free(m);
    EXPECT_TRUE(got);
    EXPECT_EQ(a, 1);
    EXPECT_EQ(b, 2);
    EXPECT_EQ(size, 2);
}

TEST(similar_and_empty_keys_are_distinct) {
    StrMap *m = strmap_new();
    EXPECT_NOT_NULL(m);
    strmap_put(m, "ab", 1);
    strmap_put(m, "ba", 2);
    strmap_put(m, "abc", 3);
    strmap_put(m, "", 4);
    int v1 = 0, v2 = 0, v3 = 0, v4 = 0, v5 = 99;
    bool got = strmap_get(m, "ab", &v1) && strmap_get(m, "ba", &v2) && strmap_get(m, "abc", &v3) && strmap_get(m, "", &v4);
    bool prefix_found = strmap_get(m, "a", &v5);
    size_t size = strmap_size(m);
    strmap_free(m);
    EXPECT_TRUE(got);
    EXPECT_EQ(v1, 1);
    EXPECT_EQ(v2, 2);
    EXPECT_EQ(v3, 3);
    EXPECT_EQ(v4, 4);
    EXPECT_FALSE(prefix_found);
    EXPECT_EQ(size, 4);
}

TEST(holds_a_thousand_keys) {
    StrMap *m = strmap_new();
    EXPECT_NOT_NULL(m);
    char key[32];
    bool ok = true;
    for (int i = 0; i < 1000; i++) {
        snprintf(key, sizeof key, "sensor.%d", i);
        ok = strmap_put(m, key, i * 7) && ok;
    }
    bool all_found = true;
    for (int i = 0; i < 1000 && all_found; i++) {
        int v = -1;
        snprintf(key, sizeof key, "sensor.%d", i);
        all_found = strmap_get(m, key, &v) && v == i * 7;
    }
    size_t size = strmap_size(m);
    strmap_free(m);   // must release every key and entry
    strmap_free(NULL);
    EXPECT_TRUE(ok);
    EXPECT_TRUE(all_found);
    EXPECT_EQ(size, 1000);
}
