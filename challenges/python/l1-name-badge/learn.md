### Strings

A **string** (`str`) is text inside quotes: `"hello"` or `'hello'`. Strings come with built-in **methods**, which are functions you call with a dot after the value:

```python
word = "  Hello There  "
word.upper()   # "  HELLO THERE  "
word.lower()   # "  hello there  "
word.strip()   # "Hello There"  (spaces at the ends removed)
```

Methods don't change the original string. They give you back a **new** one, so store it if you want to keep it: `word = word.strip()`. You can also chain them: `word.strip().lower()` gives `"hello there"`.

### Picking out characters

Each character has a position called an **index**, and counting starts at **0**:

```python
city = "Paris"
city[0]    # "P"  (the first character)
city[-1]   # "s"  (the last character)
city[:3]   # "Par" (a *slice*: from the start, up to but not including index 3)
```

Careful: `""[0]` crashes with an `IndexError`, because an empty string has no first character. Check for `""` before you index.
