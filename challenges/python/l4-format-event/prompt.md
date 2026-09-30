Your team logs events as one line of text that's easy to grep. Write a helper that takes **any number** of tags and fields.

**`format_event(name, *tags, **fields)`** returns one string:

- It starts with `name`.
- Then each tag as `#tag`, in the order given.
- Then each field as `key=value`, in the order given. The value is converted with `str()`.
- Everything is separated by single spaces.

**`format_error(*tags, **fields)`** is a shortcut that calls `format_event` with the name `"error"`, forwarding all tags and fields unchanged.

```python
format_event("deploy")                          # "deploy"
format_event("login", "web", "mobile")          # "login #web #mobile"
format_event("login", "web", user="ada", ok=True)
# "login #web user=ada ok=True"
format_error("db", table="orders", retries=3)
# "error #db table=orders retries=3"
```
