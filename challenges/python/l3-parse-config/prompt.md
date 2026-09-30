Your app is configured through a single environment variable holding a connection string like `"host=db.local; port=5432; sslmode=require"`. Write `parse_config(text)` that turns it into a `dict[str, str]`.

- Segments are separated by `;`. Empty segments (`";;"`, a trailing `;`, whitespace only) are ignored.
- Each segment is `key=value`, split at the **first** `=`, so the value may itself contain `=`.
- Whitespace around keys and values is stripped, and keys are **lowercased**.
- If a key appears twice, the **later** value wins.
- A non-empty segment **without** `=` raises `ValueError`, and the message must contain the stripped segment.

```python
parse_config("host=db.local; port=5432;")
# {"host": "db.local", "port": "5432"}

parse_config("Password = s3cr=t ; HOST=a; host=b")
# {"password": "s3cr=t", "host": "b"}

parse_config("host=db; oops ;")   # ValueError: malformed segment: 'oops'
parse_config("")                  # {}
```
