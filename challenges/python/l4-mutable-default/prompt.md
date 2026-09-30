A teammate wrote `add_tag` to attach a tag to a support ticket's tag list. It passed their one test, but in production **tickets started showing each other's tags**. Fix it.

```python
add_tag("urgent")   # ["urgent"]
add_tag("billing")  # expected ["billing"], but the buggy code gives ["urgent", "billing"]
```

What `add_tag(tag, tags)` must do:

- With a list: append `tag` to **that same list** (mutate it) and return it.
- Without a list: return a **brand-new** list `[tag]`, never shared with other calls.
- The default for `tags` must be `None`, and passing `None` explicitly behaves like passing nothing.
