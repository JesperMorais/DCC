**Bug report from the office kitchen**

> Our coffee tracker says I drank 23 cups today. I had five. Please, my doctor reads these.

The tracker has two text boxes: cups in the **morning** and cups in the **afternoon**. Text boxes always give back **strings**, like `"2"`. `cupsToday(morning, afternoon)` should return the **total number of cups** as a number.

- `cupsToday("1", "1")` → `2`
- `cupsToday("0", "4")` → `4`
- `cupsToday("6", "0")` → `6`

**Reproduction:** `cupsToday("2", "3")` returns `23`, expected `5`.

Find the bug and fix it.
