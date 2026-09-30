### Adding and changing entries

You can add a new key to a dict, or replace an existing value, by assigning to it:

```python
high_scores: dict[str, int] = {}
high_scores["ada"] = 120      # {"ada": 120}
high_scores["linus"] = 95     # {"ada": 120, "linus": 95}
high_scores["ada"] = 150      # overwrites: {"ada": 150, "linus": 95}
```

### Updating a value that might not exist

`+=` on a dict entry reads the old value first, so it **crashes** if the key isn't there yet:

```python
high_scores["ada"] += 10      # fine, ada exists (now 160)
high_scores["grace"] += 10    # KeyError: 'grace'
```

So you need to handle "first time I see this key" separately from "I've seen it before". Either check with `in` first, or read the value with `.get(key, default)`, which never crashes.

### Skipping an item

Inside a loop, `continue` jumps straight to the next item:

```python
for word in ["hi", "", "there"]:
    if word == "":
        continue
    print(word)   # "hi", then "there"
```
