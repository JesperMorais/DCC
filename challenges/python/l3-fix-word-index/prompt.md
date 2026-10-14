**Bug report from the docs team:** "We built a search box for our help pages on top of `word_index`, but every search says *no results*, even for the word *the*."

`word_index(lines)` builds a lookup from each word to the **line numbers** it appears on.

- Lines are numbered from **1**.
- Words are split on whitespace and **lowercased**, so `"Reset"` and `"reset"` are the same word.
- Line numbers are listed in order. A word that appears twice on one line lists that line twice.
- No lines (or only blank lines) gives `{}`.

```python
word_index(["Reset the router", "Call the help desk"])
# {"reset": [1], "the": [1, 2], "router": [1],
#  "call": [2], "help": [2], "desk": [2]}
```

**Reproduction:** `word_index(["hello world"])` returns `{}`, expected `{"hello": [1], "world": [1]}`.
