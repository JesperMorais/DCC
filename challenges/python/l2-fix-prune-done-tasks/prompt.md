**Bug report from the to-do app team:** "Clicking *Clear finished* usually works, but when two finished tasks are next to each other, one of them stays on the list."

`clear_finished(tasks)` removes every finished task from the list **in place** (it changes the list it's given and returns nothing). A task is finished when it starts with `"[x] "`. Open tasks start with `"[ ] "` and keep their order.

```python
tasks = ["[ ] buy milk", "[x] call mum", "[ ] fix bike"]
clear_finished(tasks)
tasks  # ["[ ] buy milk", "[ ] fix bike"]
```

- `["[x] a", "[ ] b"]` becomes `["[ ] b"]`
- `["[ ] a"]` stays `["[ ] a"]`

**Reproduction:** with `tasks = ["[x] pay rent", "[x] water plants", "[ ] book dentist"]`, after `clear_finished(tasks)` the list is `["[x] water plants", "[ ] book dentist"]`, expected `["[ ] book dentist"]`.
