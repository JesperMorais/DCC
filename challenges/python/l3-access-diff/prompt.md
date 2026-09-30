Security wants a weekly report on who can access the production database. You get the list of usernames from **before** and **after** a change. Write `access_diff(before, after)` that returns a dict with three keys:

- `"added"`: users in `after` but not in `before`
- `"removed"`: users in `before` but not in `after`
- `"kept"`: users in both

Each list is **sorted alphabetically** and contains **no duplicates**, even if the input lists repeat names. All three keys are always present, even when a list is empty.

```python
access_diff(["ada", "linus", "grace"], ["grace", "guido", "ada"])
# {"added": ["guido"], "removed": ["linus"], "kept": ["ada", "grace"]}

access_diff([], [])
# {"added": [], "removed": [], "kept": []}
```
