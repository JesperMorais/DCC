### Struct assignment copies

In C, assigning a struct copies **every member** into a new, independent object. That's true for function arguments, return values, and plain local variables:

```c
typedef struct { int x, y; } Point;

Point points[2] = {{1, 2}, {3, 4}};
Point p = points[0];   // a copy
p.x = 99;              // points[0].x is still 1
```

If you want to change the original, take its address and work through a pointer:

```c
Point *q = &points[0];
q->x = 99;             // points[0].x is now 99
```

Copies are great when you want a snapshot or when the function only reads. A pointer is what you need when the change has to outlive the current scope. Both compile without a single warning, so the compiler won't tell you which one you meant.

### Debugging: watch the object, not the variable

When an update "doesn't stick", print the **original** right after the line that should have changed it, not the local variable you changed. If the local is correct and the original isn't, you are holding a copy. A debugger watch on `&w->items[0]` versus `&item` makes the two addresses visible.
