An import job reads a spreadsheet export that has already been split into cells. Write `rows_to_dicts(header, rows)` that turns each row into a dict keyed by column name.

- Column names are cleaned: surrounding whitespace is stripped and they're **lowercased**.
- Cell values have surrounding whitespace stripped.
- A row with a **different number of cells** than the header is malformed and is **skipped**.
- Records keep the order of the rows.

```python
header = ["Name", " Email "]
rows = [["Ada ", "ada@example.com"], ["Linus"], ["Grace", " grace@example.com"]]

rows_to_dicts(header, rows)
# [{"name": "Ada", "email": "ada@example.com"},
#  {"name": "Grace", "email": "grace@example.com"}]

rows_to_dicts(header, [])  # []
```
