A teacher turns test scores (0–100) into letter grades like this:

| Score        | Grade |
|--------------|-------|
| 90 or more   | `"A"` |
| 80 – 89      | `"B"` |
| 70 – 79      | `"C"` |
| 60 – 69      | `"D"` |
| below 60     | `"F"` |

Write a function `letterGrade(score)` that returns the grade as a string.

- `letterGrade(95)` → `"A"`
- `letterGrade(80)` → `"B"` (exactly 80 is a B)
- `letterGrade(72)` → `"C"`
- `letterGrade(12)` → `"F"`
