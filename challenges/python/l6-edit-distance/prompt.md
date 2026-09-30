"Did you mean *python*?" Spell-checkers rank suggestions by **edit distance**. Write **`edit_distance`**, the Levenshtein distance between two strings:

```python
def edit_distance(a: str, b: str) -> int
```

It's the minimum number of single-character edits that turn `a` into `b`, where an edit is one of:

- **insert** a character,
- **delete** a character,
- **substitute** one character for another.

Comparison is case-sensitive. It must be fast enough for strings of a few hundred characters: the tests compare two 400-character strings, well inside the time limit.

```python
edit_distance("kitten", "sitting")   # 3: k→s, e→i, +g
edit_distance("flaw", "lawn")        # 2: -f, +n
edit_distance("", "abc")             # 3
edit_distance("same", "same")        # 0
```
