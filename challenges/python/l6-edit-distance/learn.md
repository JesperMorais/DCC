### Dynamic programming = recursion + remembering

Many "minimum/maximum/how many ways" problems have a recursive definition in which the **same sub-questions come up again and again**. Example: the cheapest path from the top-left to the bottom-right of a cost grid, moving only right or down:

```python
def cheapest(grid: list[list[int]]) -> int:
    @cache
    def best(r: int, c: int) -> int:            # cheapest cost to reach (r, c)
        here = grid[r][c]
        if r == 0 and c == 0:
            return here
        options = []
        if r > 0: options.append(best(r - 1, c))   # came from above
        if c > 0: options.append(best(r, c - 1))   # came from the left
        return here + min(options)
    return best(len(grid) - 1, len(grid[0]) - 1)
```

Without `@cache`, `best` re-solves the same cells an exponential number of times. With it, each `(r, c)` is solved **once**, so the cost is rows × cols.

### The recipe

1. **Define the subproblem precisely.** "Answer for the first `i` items of this and the first `j` of that" is a very common shape for two-sequence problems.
2. **Write the recurrence.** How does the answer for `(i, j)` follow from *smaller* subproblems? Consider each possible "last move".
3. **Base cases**, when one side is empty.
4. **Evaluate**, top-down with `@functools.cache`, or bottom-up by filling a table in an order where every dependency is already computed.

Bottom-up needs no recursion (so no recursion-depth limit), and often only the **previous row** of the table has to be kept.
