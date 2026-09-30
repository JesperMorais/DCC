### Parse in layers

Structured strings are easiest to parse **from the outside in**: split into records, then split each record into fields. Each layer stays small and easy to test.

```python
text = "red:3, blue:5"
for item in text.split(","):
    colour, count = item.split(":")
```

### `split` with a limit

`str.split(sep, maxsplit)` stops after `maxsplit` splits, and the rest stays in the last piece:

```python
"a=b=c".split("=")      # ["a", "b", "c"]
"a=b=c".split("=", 1)   # ["a", "b=c"]
" x ".strip()           # "x"
```

### Skipping blanks

Splitting `"a;;b;"` on `";"` gives `["a", "", "b", ""]`. After `strip()`, an empty string is **falsy**, so `if not piece: continue` skips it.

### Failing loudly

When input is broken, raise an error that **shows the bad part**. `!r` in an f-string adds quotes, so whitespace and empty values stay visible:

```python
raise ValueError(f"bad item: {item!r}")  # ValueError: bad item: 'blue'
```
