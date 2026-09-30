#include <stdbool.h>
#include <stddef.h>

typedef struct {
    int x, y;   // top-left corner
    int w, h;   // width and height
} Rect;

static int max_int(int a, int b) { return a > b ? a : b; }
static int min_int(int a, int b) { return a < b ? a : b; }

int rect_area(const Rect *r) {
    return r->w * r->h;
}

bool rect_intersect(Rect a, Rect b, Rect *out) {
    int left = max_int(a.x, b.x);
    int top = max_int(a.y, b.y);
    int right = min_int(a.x + a.w, b.x + b.w);
    int bottom = min_int(a.y + a.h, b.y + b.h);
    if (right <= left || bottom <= top) {
        return false;
    }
    if (out != NULL) {
        *out = (Rect){ left, top, right - left, bottom - top };
    }
    return true;
}
