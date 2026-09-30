A music app needs a small model for playlists. Finish the `Playlist` **dataclass**:

- Fields: `name: str` and `tracks: list[int]`, where each track is a duration in **seconds**. `tracks` is optional and defaults to an empty list. Every playlist must get **its own** list.
- `add(seconds)` appends one track.
- `duration()` returns the total length as `"m:ss"`. Minutes aren't capped at 59, and seconds are always two digits.

Because it's a dataclass, `==` and `repr` come for free. The tests check those too.

```python
p = Playlist("Focus")
p.add(185)
p.add(200)
p.duration()                                  # "6:25"
Playlist("Empty").duration()                  # "0:00"
Playlist("Mix", [30, 30]) == Playlist("Mix", [30, 30])  # True
```
