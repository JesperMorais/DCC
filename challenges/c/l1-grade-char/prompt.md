A school's report-card printer needs one letter per subject. Write `grade_letter`, which turns a test score (0–100) into a letter grade:

```c
char grade_letter(int score);
```

| Score      | Grade |
|------------|-------|
| 90 or more | `'A'` |
| 80 – 89    | `'B'` |
| 70 – 79    | `'C'` |
| 60 – 69    | `'D'` |
| below 60   | `'F'` |

Examples:

- `grade_letter(95)` → `'A'`
- `grade_letter(80)` → `'B'` (exactly 80 is a B)
- `grade_letter(72)` → `'C'`
- `grade_letter(12)` → `'F'`
