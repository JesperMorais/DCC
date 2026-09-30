Config files are nested, but environment variables and log lines are flat. Write **`flatten_config`**, which turns a nested dict into a flat one with **dotted keys**.

```python
def flatten_config(config: Mapping[str, object], sep: str = ".") -> dict[str, object]
```

- A value that is itself a `dict` is flattened recursively: its keys are joined to the parent key with `sep`.
- Every other value (numbers, strings, `None`, **lists**) is a leaf and is copied as-is. Don't look inside lists.
- An empty nested dict has no leaves, so it produces no keys.
- Nesting can be any depth. Don't modify the input.

```python
flatten_config({"db": {"host": "localhost", "port": 5432}, "debug": True})
# {"db.host": "localhost", "db.port": 5432, "debug": True}

flatten_config({"a": {"b": {"c": 1}}}, sep="__")
# {"a__b__c": 1}

flatten_config({"tags": ["x", "y"], "extra": {}})
# {"tags": ["x", "y"]}
```
