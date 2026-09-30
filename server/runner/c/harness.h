/* daily.ts C test harness — included automatically before your code and the tests.
 *
 *   TEST(adds_two_numbers) {
 *       EXPECT_EQ(add(2, 3), 5);
 *   }
 *
 * Matchers stop the test at the first failure:
 *   EXPECT_EQ(a, e) / EXPECT_NE(a, e)      integers (compared as long long)
 *   EXPECT_UEQ(a, e)                        unsigned integers (unsigned long long)
 *   EXPECT_TRUE(c) / EXPECT_FALSE(c)
 *   EXPECT_NEAR(a, e, eps)                  doubles
 *   EXPECT_STR_EQ(a, e)                     C strings (NULL-safe)
 *   EXPECT_NULL(p) / EXPECT_NOT_NULL(p) / EXPECT_PTR_EQ(a, e)
 *   EXPECT_INT_ARRAY_EQ(a, e, n)            int arrays of length n
 */
#ifndef DTS_HARNESS_H
#define DTS_HARNESS_H

#include <stdbool.h>
#include <stddef.h>

typedef void (*dts_test_fn)(void);
void dts_register(const char *name, dts_test_fn fn, int line);
void dts_fail(int line, const char *expr, const char *expected, const char *received);
void dts_fail_ll(int line, const char *expr, long long expected, long long received);
void dts_fail_ull(int line, const char *expr, unsigned long long expected, unsigned long long received);
void dts_fail_dbl(int line, const char *expr, double expected, double received);
void dts_fail_str(int line, const char *expr, const char *expected, const char *received);
void dts_fail_ptr(int line, const char *expr, const void *expected, const void *received);
void dts_fail_arr(int line, const char *expr, const int *expected, const int *received, size_t n);
bool dts_str_eq(const char *a, const char *b);
bool dts_arr_eq(const int *a, const int *b, size_t n);

#define TEST(name)                                                                              \
    static void name(void);                                                                     \
    __attribute__((constructor)) static void dts_reg_##name(void) { dts_register(#name, name, __LINE__); } \
    static void name(void)

#define EXPECT_EQ(actual, expected)                                                             \
    do {                                                                                        \
        long long dts_a_ = (long long)(actual), dts_e_ = (long long)(expected);                 \
        if (dts_a_ != dts_e_) { dts_fail_ll(__LINE__, #actual " == " #expected, dts_e_, dts_a_); return; } \
    } while (0)

#define EXPECT_NE(actual, expected)                                                             \
    do {                                                                                        \
        long long dts_a_ = (long long)(actual), dts_e_ = (long long)(expected);                 \
        if (dts_a_ == dts_e_) { dts_fail_ll(__LINE__, #actual " != " #expected, dts_e_, dts_a_); return; } \
    } while (0)

#define EXPECT_UEQ(actual, expected)                                                            \
    do {                                                                                        \
        unsigned long long dts_a_ = (unsigned long long)(actual), dts_e_ = (unsigned long long)(expected); \
        if (dts_a_ != dts_e_) { dts_fail_ull(__LINE__, #actual " == " #expected, dts_e_, dts_a_); return; } \
    } while (0)

#define EXPECT_TRUE(cond)                                                                       \
    do { if (!(cond)) { dts_fail(__LINE__, #cond, "true", "false"); return; } } while (0)

#define EXPECT_FALSE(cond)                                                                      \
    do { if (cond) { dts_fail(__LINE__, "!(" #cond ")", "false", "true"); return; } } while (0)

#define EXPECT_NEAR(actual, expected, eps)                                                      \
    do {                                                                                        \
        double dts_a_ = (double)(actual), dts_e_ = (double)(expected);                          \
        double dts_d_ = dts_a_ - dts_e_;                                                        \
        if (dts_d_ < 0) dts_d_ = -dts_d_;                                                       \
        if (!(dts_d_ <= (eps))) { dts_fail_dbl(__LINE__, #actual " ≈ " #expected, dts_e_, dts_a_); return; } \
    } while (0)

#define EXPECT_STR_EQ(actual, expected)                                                         \
    do {                                                                                        \
        const char *dts_a_ = (actual), *dts_e_ = (expected);                                    \
        if (!dts_str_eq(dts_a_, dts_e_)) { dts_fail_str(__LINE__, #actual " == " #expected, dts_e_, dts_a_); return; } \
    } while (0)

#define EXPECT_NULL(ptr)                                                                        \
    do { const void *dts_p_ = (const void *)(ptr); if (dts_p_ != NULL) { dts_fail_ptr(__LINE__, #ptr " == NULL", NULL, dts_p_); return; } } while (0)

#define EXPECT_NOT_NULL(ptr)                                                                    \
    do { if ((const void *)(ptr) == NULL) { dts_fail(__LINE__, #ptr " != NULL", "a non-NULL pointer", "NULL"); return; } } while (0)

#define EXPECT_PTR_EQ(actual, expected)                                                         \
    do {                                                                                        \
        const void *dts_a_ = (const void *)(actual), *dts_e_ = (const void *)(expected);        \
        if (dts_a_ != dts_e_) { dts_fail_ptr(__LINE__, #actual " == " #expected, dts_e_, dts_a_); return; } \
    } while (0)

#define EXPECT_INT_ARRAY_EQ(actual, expected, n)                                                \
    do {                                                                                        \
        const int *dts_a_ = (actual), *dts_e_ = (expected);                                     \
        size_t dts_n_ = (size_t)(n);                                                            \
        if (!dts_arr_eq(dts_a_, dts_e_, dts_n_)) { dts_fail_arr(__LINE__, #actual " == " #expected, dts_e_, dts_a_, dts_n_); return; } \
    } while (0)

#endif
