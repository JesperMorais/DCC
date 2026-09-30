#include <stdbool.h>
#include <stddef.h>

typedef struct {
    int x, y;   // top-left corner
    int w, h;   // width and height
} Rect;

int rect_area(const Rect *r) {
    (void)r;
    return 0;
}

bool rect_intersect(Rect a, Rect b, Rect *out) {
    (void)a;
    (void)b;
    (void)out;
    return false;
}
