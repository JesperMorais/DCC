A window manager only redraws the part of the screen where two windows overlap. Rectangles use screen coordinates: `(x, y)` is the **top-left** corner, `y` grows downwards, and `w`/`h` are never negative.

```c
typedef struct {
    int x, y;   // top-left corner
    int w, h;   // width and height
} Rect;

int  rect_area(const Rect *r);
bool rect_intersect(Rect a, Rect b, Rect *out);
```

- `rect_area` returns `w * h`. It takes a pointer because it only needs to *look* at the rectangle.
- `rect_intersect` returns `true` if the rectangles share some area, and stores the overlapping rectangle in `*out`.
- Rectangles that only **touch** along an edge (or a corner) don't overlap: return `false`.
- When there's no overlap, **leave `*out` unchanged**.
- `out` may be `NULL` when the caller only wants the yes/no answer.

```c
Rect a = {0, 0, 10, 10}, b = {5, 5, 10, 10}, hit;
rect_intersect(a, b, &hit);   // → true, hit = {5, 5, 5, 5}
rect_intersect(a, (Rect){10, 0, 5, 5}, &hit);   // → false (they just touch)
rect_area(&a);                // → 100
```

The `Rect` type is already defined in the starter.
