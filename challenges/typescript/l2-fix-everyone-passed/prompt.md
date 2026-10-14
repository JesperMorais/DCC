**Bug report from a maths teacher**

> The report says the whole class passed the test, so I sent a "well done, everyone!" email. Then Sam showed me their 40.

`everyonePassed(scores, passMark)` returns `true` if **every** score is at least `passMark`, and `false` if **any** score is below it. A class with no scores yet counts as `true` (nobody failed).

- `everyonePassed([70, 85, 50], 50)` → `true`
- `everyonePassed([30, 90], 50)` → `false`
- `everyonePassed([], 50)` → `true`

**Reproduction:** `everyonePassed([80, 40, 90], 50)` returns `true`, expected `false`.

Find the bug and fix it.
