Tests often need to flip a feature flag "just for this block". Write a context manager **`override`** that temporarily changes entries in a settings dict and **always** puts them back.

```python
@contextmanager
def override(settings: dict[str, object], **changes: object) -> Iterator[dict[str, object]]
```

- On entering the `with` block, apply `changes` to `settings` **in place**, and bind `settings` to the `as` target.
- On leaving, restore every overridden key to its previous value. A key that **didn't exist** before is removed again. Note that a stored value of `None` is still a value.
- Restore even when the block raises, but don't swallow the exception.
- Only the overridden keys are restored. Other edits made inside the block stay.
- Overrides can be nested.

```python
flags = {"dark_mode": False, "beta": None}
with override(flags, dark_mode=True, new_ui=True) as f:
    f["dark_mode"], f["new_ui"]    # True, True
flags                              # {"dark_mode": False, "beta": None}
```
