**Bug report from the social team:** "The trending widget shows `#Python` and `#python` as two separate trends, each with a third of the posts. But I checked: `Hashtag('#Python') == Hashtag('#python')` is `True`! How can they be equal and still counted apart?"

A `Hashtag` ignores a leading `#`, surrounding spaces and **letter case**. Two hashtags that compare equal must be treated as the same tag everywhere: in `==`, in sets, as dict keys, and when counting.

- `unique_hashtags(tags)` returns the set of distinct hashtags.
- `trending(posts, top)` counts every hashtag across all posts and returns the `top` most common as `(text, count)`, using the spelling seen first.
- `Hashtag("#")` raises `ValueError`.

```python
Hashtag("#fika") == Hashtag(" fika ")                    # True
unique_hashtags(["#fika", "#fika", "#kanelbulle"])      # {Hashtag('#fika'), Hashtag('#kanelbulle')}
```

**Reproduction:**

```python
len(unique_hashtags(["#Python", "#python", "#PYTHON", "#rust"]))
# returns 4, expected 2
```
