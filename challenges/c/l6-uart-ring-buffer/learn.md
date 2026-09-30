### A queue that never moves its data

A naive array queue shifts every element down on each pop, which is O(n). A **ring buffer** leaves the data where it is and moves two indices instead: one where you read, one where you write. When an index runs off the end of the array, it wraps back to 0, as if the array's ends were glued together in a circle.

```
data:  [ c ][ d ][ . ][ a ][ b ]
                   ▲    ▲
                 tail  head      reading order: a, b, c, d
```

Wrapping is one line of modular arithmetic:

```c
i = (i + 1) % SIZE;   // SIZE - 1 → 0
```

### Empty or full?

If you store only the two indices, "empty" and "full" look the same: in both cases they're equal. Common fixes are to keep one slot unused, or to track a **count** alongside the indices. The count version uses every slot and makes both checks trivial.

### Why embedded code loves this

It's fixed-size (no heap, no fragmentation), O(1) per operation, and with a single producer and a single consumer each index is written by only one side. That property is the basis of lock-free UART, audio and logging buffers.
