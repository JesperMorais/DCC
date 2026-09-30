### The off-by-one error

An array of `n` elements has indices `0, 1, …, n - 1`. There is **no** element `n`:

```c
int lap_times[3] = {61, 59, 60};
lap_times[0];   // 61, the first
lap_times[2];   // 60, the last
lap_times[3];   // ✗ one past the end: undefined behaviour
```

Being off by one in a loop bound is probably the most common bug in all of programming. It has a name: the **fencepost error**. (A 10-metre fence with a post every metre needs 11 posts, not 10.)

### Why C doesn't stop you

C doesn't check array bounds. `lap_times[3]` just reads whatever bytes happen to sit after the array in memory: another variable, leftover junk, sometimes a value that makes the answer look right. The program may "work" for months and then misbehave on a different compiler or input.

That's why daily.ts compiles your code with **AddressSanitizer**. It puts invisible guard zones around every array, and the moment you touch one, it stops the program and reports *where* the bad access happened.

### Checking a loop by hand

For any loop over an array, ask two questions:

1. What is `i` on the **first** round?
2. What is `i` on the **last** round, and is every index I use then still `< n`?

If the body uses `i - 1` or `i + 1`, check those too.
