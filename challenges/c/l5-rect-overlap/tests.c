#include <stdbool.h>
#include <stddef.h>

TEST(area_reads_through_the_pointer) {
    Rect r = {3, 4, 20, 5};
    EXPECT_EQ(rect_area(&r), 100);
    Rect empty = {0, 0, 0, 7};
    EXPECT_EQ(rect_area(&empty), 0);
}

TEST(partial_overlap) {
    Rect a = {0, 0, 10, 10}, b = {5, 5, 10, 10};
    Rect hit = {0, 0, 0, 0};
    EXPECT_TRUE(rect_intersect(a, b, &hit));
    EXPECT_EQ(hit.x, 5);
    EXPECT_EQ(hit.y, 5);
    EXPECT_EQ(hit.w, 5);
    EXPECT_EQ(hit.h, 5);
}

TEST(one_window_inside_another) {
    Rect screen = {0, 0, 1920, 1080}, dialog = {800, 400, 320, 200};
    Rect hit = {0, 0, 0, 0};
    EXPECT_TRUE(rect_intersect(screen, dialog, &hit));
    EXPECT_EQ(hit.x, 800);
    EXPECT_EQ(hit.y, 400);
    EXPECT_EQ(hit.w, 320);
    EXPECT_EQ(hit.h, 200);
    EXPECT_EQ(rect_area(&hit), 64000);
}

TEST(order_of_arguments_does_not_matter) {
    Rect a = {-5, 2, 10, 4}, b = {0, -10, 3, 14};
    Rect ab = {0, 0, 0, 0}, ba = {0, 0, 0, 0};
    EXPECT_TRUE(rect_intersect(a, b, &ab));
    EXPECT_TRUE(rect_intersect(b, a, &ba));
    EXPECT_EQ(ab.x, 0);
    EXPECT_EQ(ab.y, 2);
    EXPECT_EQ(ab.w, 3);
    EXPECT_EQ(ab.h, 2);
    EXPECT_EQ(ba.x, ab.x);
    EXPECT_EQ(ba.y, ab.y);
    EXPECT_EQ(ba.w, ab.w);
    EXPECT_EQ(ba.h, ab.h);
}

TEST(disjoint_or_touching_is_false_and_out_is_untouched) {
    Rect a = {0, 0, 10, 10};
    Rect hit = {7, 7, 7, 7};
    EXPECT_FALSE(rect_intersect(a, (Rect){50, 50, 5, 5}, &hit));   // far apart
    EXPECT_FALSE(rect_intersect(a, (Rect){10, 0, 5, 5}, &hit));    // shares the right edge
    EXPECT_FALSE(rect_intersect(a, (Rect){0, 10, 5, 5}, &hit));    // shares the bottom edge
    EXPECT_FALSE(rect_intersect(a, (Rect){10, 10, 5, 5}, &hit));   // shares only a corner
    EXPECT_EQ(hit.x, 7);
    EXPECT_EQ(hit.y, 7);
    EXPECT_EQ(hit.w, 7);
    EXPECT_EQ(hit.h, 7);
}

TEST(null_out_just_answers_yes_or_no) {
    Rect a = {0, 0, 10, 10};
    EXPECT_TRUE(rect_intersect(a, (Rect){9, 9, 5, 5}, NULL));
    EXPECT_FALSE(rect_intersect(a, (Rect){20, 0, 5, 5}, NULL));
}
