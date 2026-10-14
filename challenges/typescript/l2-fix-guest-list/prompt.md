**Bug report from the party door**

> Ana organised the party and she's first on the list. The door app turned her away. Meanwhile it let in a guy called Brad, who nobody invited.

`isOnGuestList(guests, name)` returns `true` if `name` is on the guest list and `false` if it isn't. Names are matched exactly.

- `isOnGuestList(["Ana", "Leo", "Kim"], "Leo")` → `true`
- `isOnGuestList(["Ana", "Leo", "Kim"], "Brad")` → `false`
- `isOnGuestList([], "Ana")` → `false`

**Reproduction:** `isOnGuestList(["Ana", "Leo", "Kim"], "Ana")` returns `false`, expected `true`.

Find the bug and fix it.
