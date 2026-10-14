**Bug report from the arcade**

> I scored 100 on Space Blaster and my name isn't on the high-score screen. Someone with 9 points is in first place.

`topThree(scores)` returns the **three highest** scores, biggest first. If there are fewer than three scores, it returns all of them, biggest first. It must not change the array it was given.

- `topThree([5, 3, 9, 1])` → `[9, 5, 3]`
- `topThree([4, 7])` → `[7, 4]`
- `topThree([])` → `[]`

**Reproduction:** `topThree([100, 25, 9, 80])` returns `[9, 80, 25]`, expected `[100, 80, 25]`.

Find the bug and fix it.
