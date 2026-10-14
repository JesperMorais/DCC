### Sets for fast membership

A `set` holds each value at most once and answers "is this in here?" with `in`, very quickly. They're perfect for "have I seen this?" and "is this blocked?" checks:

```python
blocked = {"spam@bad.io", "bot@bad.io"}
"bot@bad.io" in blocked    # True
```

### Normalising before comparing

`in` and `==` compare **exactly**. For strings, `"A"` and `"a"` are different, and so are `"a"` and `" a"`. When two values should count as equal even though they're written differently (emails, usernames, tags), turn both into one **canonical form** before comparing:

```python
def canonical(tag: str) -> str:
    return tag.strip().lower()

seen = {canonical(t) for t in ["Python", "rust "]}
canonical(" PYTHON") in seen   # True
```

The rule is: **every** value that takes part in a comparison has to go through the same normalisation. Missing it on one side is enough to break the match.

### Debugging technique: shrink the failing input

Start from the failing example and remove things until it's as small as possible while still failing. Then change one detail at a time (capitals, spaces, order) and see which change makes the bug appear or disappear.
