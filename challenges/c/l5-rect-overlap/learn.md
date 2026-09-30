### Structs group data

A `struct` bundles related values into one type. `typedef` gives it a short name:

```c
typedef struct {
    double lat, lon;
} GeoPoint;

GeoPoint home = { 59.33, 18.07 };   // positional initializer
GeoPoint cafe = { .lat = 59.34, .lon = 18.06 };   // designated initializer
```

### By value or by pointer?

Passing a struct **by value** copies the whole thing. The function gets its own copy, so it can't change the caller's struct:

```c
double lat_of(GeoPoint p) { return p.lat; }        // p is a copy
```

Passing **by pointer** copies only an address. It's cheaper for big structs, and it lets the function write back. Use `->` to reach a field through a pointer (it's shorthand for `(*p).field`):

```c
void nudge_north(GeoPoint *p) { p->lat += 0.01; }  // changes the caller's struct
double lat_of2(const GeoPoint *p) { return p->lat; } // const: read-only access
```

A **compound literal** builds a struct value on the spot: `*p = (GeoPoint){ 1.0, 2.0 };`.

### Geometry tip

Two-dimensional problems often split into two one-dimensional ones. Work out what's true on the x-axis, then the y-axis, and combine the answers.
