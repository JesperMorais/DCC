The maths department exports its marks from a spreadsheet, and the teachers want a proper grade book instead of a wall of formulas. Each student arrives as a `Student` (see the starter). Names are typed by hand, so some have stray spaces around them.

**Step 1: the types.** Replace the two placeholders:

- `Grade` is exactly one of `"A"`, `"B"`, `"C"`, `"D"`, `"F"`.
- `ReportCard` has three properties and nothing else: `name` (string), `average` (number) and `grade` (a `Grade`).

**Step 2: the functions.**

**`averageScore(scores)`** returns the mean, rounded to **one decimal**. An empty list gives `0`.

```ts
averageScore([90, 93]);     // 91.5
averageScore([61, 62, 62]); // 61.7
averageScore([]);           // 0
```

**`gradeFor(average)`** turns an average into a grade:

| Average | Grade |
|---|---|
| 90 or more | `"A"` |
| 75 – below 90 | `"B"` |
| 60 – below 75 | `"C"` |
| 50 – below 60 | `"D"` |
| below 50 | `"F"` |

**`reportCard(student)`** returns `{ name, average, grade }`. The name is trimmed, `average` comes from `averageScore`, and `grade` is the grade **for that rounded average**.

```ts
reportCard({ name: "  Ada ", scores: [90, 93] }); // { name: "Ada", average: 91.5, grade: "A" }
```

**`honorRoll(students)`** returns the trimmed names of every student whose grade is `"A"`, in the original order.

**`curve(student, points)`** returns a **new** student with `points` added to every score, but no score may go above `100`. The name stays exactly as it was. The original student and their `scores` array must not change.

```ts
curve({ name: "Linus", scores: [70, 98] }, 5); // { name: "Linus", scores: [75, 100] }
```
