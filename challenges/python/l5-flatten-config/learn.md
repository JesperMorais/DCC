### Recursion: solve a smaller copy of the same problem

A recursive function calls itself on a *smaller piece* of its input, and stops at a **base case** that it can answer directly. Nested data is the natural fit, because a nested dict is just "a dict whose values might be dicts".

```python
def depth(tree: object) -> int:
    if not isinstance(tree, dict) or not tree:   # base case: a leaf
        return 0
    return 1 + max(depth(child) for child in tree.values())

depth({"a": {"b": {}}, "c": 1})  # 2
```

Two questions design every recursive function:

1. **What's the base case?** Here: anything that isn't a (non-empty) dict.
2. **How do I combine the sub-results?** Here: `1 + max(...)`.

### Carry context down as a parameter

Often a sub-call needs to know *where it is*, such as its path from the root. Pass that down as an extra argument (with a default, or in a small inner helper), rather than rebuilding it afterwards:

```python
def paths(tree: dict[str, object], prefix: str = "") -> list[str]:
    ...  # each child call receives prefix + its own key
```

### `isinstance` narrows types

After `if isinstance(value, dict):`, mypy knows `value` is a dict inside that branch, so calling `.items()` on it type-checks.
