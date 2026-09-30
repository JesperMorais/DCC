Your front end gets a user profile as JSON from the API. Older accounts are missing whole sections, and some sections come back as `null`. Write `ui_prefs(profile)` that returns the user's `(theme, font_size)` as a tuple.

- The values live at `profile["settings"]["ui"]["theme"]` and `profile["settings"]["ui"]["font_size"]`.
- A missing theme defaults to `"light"`, and a missing font size defaults to `14`.
- A section (`settings` or `ui`) that is **missing or `None`** counts as empty. It must never raise.
- Each key defaults on its own, so a profile with only a theme keeps that theme and gets font size `14`.
- Only read the profile. Don't add or change any keys in it.

```python
ui_prefs({"settings": {"ui": {"theme": "dark", "font_size": 16}}})  # ("dark", 16)
ui_prefs({"settings": {"ui": {"theme": "dark"}}})                   # ("dark", 14)
ui_prefs({"settings": None})                                        # ("light", 14)
ui_prefs({})                                                        # ("light", 14)
```
