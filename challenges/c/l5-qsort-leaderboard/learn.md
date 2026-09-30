### Generic code in C: `void *` and a function

`qsort` can sort *any* array because it only needs three things from you: where the array is, how big each element is, and how to compare two elements:

```c
void qsort(void *base, size_t count, size_t size,
           int (*compare)(const void *, const void *));
```

The comparator gets pointers to two elements and returns **negative** if the first belongs before the second, **positive** if after, and **0** if they're equivalent. Inside, you turn the `void *` back into the real type:

```c
static int by_length(const void *pa, const void *pb) {
    const Box *a = pa, *b = pb;   // void * converts implicitly in C
    if (a->length < b->length) return -1;
    if (a->length > b->length) return 1;
    return 0;
}

qsort(boxes, n, sizeof *boxes, by_length);
```

Notice that you pass the function's **name** without parentheses. That hands `qsort` a pointer to the function, and it calls you back as many times as it needs to.

### The subtraction trap

`return a->length - b->length;` is a popular shortcut, and it's wrong for `int`. If one value is very large and the other very negative, the subtraction overflows. Signed overflow is **undefined behavior** in C: the compiler may assume it never happens, and UBSan will stop the program when it does. Compare, don't subtract.
