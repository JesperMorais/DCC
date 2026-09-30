### Functions have addresses too

A function's code lives in memory like everything else, so you can store a pointer to it, pass it around and call through it. The declaration syntax looks odd at first. Read it inside-out:

```c
double (*scale)(double);   // scale is a pointer to a function taking a double, returning a double
```

The parentheses around `*scale` matter. Without them, `double *scale(double);` declares a function that *returns* a `double *`.

```c
static double half(double x) { return x / 2; }

double (*scale)(double) = half;   // a function's name decays to its address
double y = scale(10.0);           // call through the pointer: 5.0
```

A `typedef` makes signatures readable when they show up in several places:

```c
typedef double (*Scaler)(double);
void apply_all(double *v, size_t n, Scaler s);
```

### Why bother?

It separates **how to walk the data** from **what to do with each item**. `qsort`, `bsearch`, signal handlers, GUI callbacks and interrupt vector tables all work this way. C has no closures, so anything else a callback needs has to come in through its parameters, or through an extra `void *context` argument, which is the pattern many C APIs use.
