Your search API returns results **one page at a time**. Write `paginate(items, page, per_page)` that returns the items on the requested page.

- Pages are numbered from **1**.
- The last page may be shorter than `per_page`.
- A page past the end gives `[]` (not an error).
- `page` or `per_page` below 1 raises `ValueError` with a message containing `"at least 1"`.
- Don't change the list you were given.

```python
results = ["a", "b", "c", "d", "e"]
paginate(results, 1, 2)  # ["a", "b"]
paginate(results, 3, 2)  # ["e"]
paginate(results, 4, 2)  # []
paginate(results, 0, 2)  # ValueError: page must be at least 1
```
