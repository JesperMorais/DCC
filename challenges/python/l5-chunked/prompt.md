Bulk-inserting rows into a database is fastest in batches, but the rows come from a stream that may be huge, or never end. Write a **generator** `chunked` that groups any iterable into lists of `n` items, **lazily**.

```python
def chunked[T](iterable: Iterable[T], n: int) -> Iterator[list[T]]
```

- Yield lists of `n` consecutive items. The **last** chunk may be shorter, and there's never an empty chunk.
- Be lazy: pull items from `iterable` only when the next chunk is requested, and only as many as that chunk needs. It must work on **infinite** iterators.
- Accept any iterable: lists, strings, generators…
- Raise `ValueError` if `n < 1` (it's fine if that happens when iteration starts).

```python
list(chunked([1, 2, 3, 4, 5, 6, 7], 3))    # [[1, 2, 3], [4, 5, 6], [7]]
list(chunked("abcde", 2))                  # [["a", "b"], ["c", "d"], ["e"]]

from itertools import count, islice
list(islice(chunked(count(), 2), 3))       # [[0, 1], [2, 3], [4, 5]]
```
