In August 2020, England cancelled its A-level exams and computed grades with an algorithm instead. Around 39% of the grades teachers had predicted came out lower, students protested outside the Department for Education, and within a week the government threw the results out. Grading code is small, but people's futures run through it, so it has to be right and it has to be readable.

This boss is a grade book. You won't learn anything new here. You'll put everything from Fundamentals together in one realistic task.

### The toolbox so far

Every piece of the grade book uses something you've already practised:

| You need to… | Reach for | Node |
|---|---|---|
| hold a number, compute an average | `const`, `+`, `/`, `Math.round` | Values |
| clean up a name with stray spaces | `.trim()` | Strings |
| turn a number into a letter | an `if / else if` chain | Decisions |
| add up a list of scores | a `for...of` loop with an accumulator | Loops |
| change scores without touching the original | build a **new** array | Arrays |
| describe a student and a grade | `type`, object types, literal unions | Objects |
| pick out and reshape students | `filter`, `map`, arrow functions | Callbacks |

### Break the task into small functions

Big tasks feel hard when you try to hold all of them in your head at once. The trick professionals use is to write **one small function per idea**, each of which you can test on its own, and then build the bigger ones out of the smaller ones:

```
scores ──► averageScore ──► gradeFor ──┐
                                       ├──► reportCard ──► honorRoll
name ─────────► trim ──────────────────┘
```

When `reportCard` is wrong, you check `averageScore` and `gradeFor` first. They're three lines each, so a bug has nowhere to hide.

### Worked example: a tiny attendance book

Here is the same pattern on a different problem, so you can see the shape without the answers:

```ts
type Mark = "present" | "late" | "absent";
type Pupil = { name: string; marks: Mark[] };

function lateCount(marks: Mark[]): number {
  let count = 0;
  for (const m of marks) {
    if (m === "late") count++;
  }
  return count;
}

function needsChat(pupils: Pupil[]): string[] {
  return pupils
    .filter((p) => lateCount(p.marks) >= 3)
    .map((p) => p.name.trim());
}
```

`Mark` is a literal union, so `"lat"` is a compile error. `lateCount` is a loop with an accumulator. `needsChat` is a `filter` then a `map`, built on top of `lateCount`. Your grade book has the same structure, with one more step in the middle.

### Rounding to one decimal

`Math.round` only rounds to whole numbers. To keep one decimal, scale up, round, and scale back:

```ts
Math.round(86.666 * 10) / 10; // 866.66 → 867 → 86.7
```

### Gotchas to check before you submit

- **Empty lists.** The average of `[]` is `0 / 0`, which is `NaN`. Decide what to return *before* you divide.
- **Boundaries.** Is exactly 90 an A? Test the boundary values, not just the easy middle ones.
- **Round once, at the end.** Round the average, then pick the grade from the rounded number, so the report card never shows `89.96` next to an A.
- **Don't modify the input.** "Add 5 points to every score" means return a new student with a new `scores` array. `student.scores[i] += 5` changes the teacher's original data.
- **`for...of`, not `for...in`.** `for (const s in scores)` gives you the indices as strings (`"0"`, `"1"`…), so `total += s` glues strings together.

### In the wild

- **School systems** like Canvas, Moodle and Google Classroom have exactly these functions at their core: averages, grade bands, curving, and honour-roll lists.
- **Payroll and invoicing** follow the same shape: a list of records, a few small calculation functions, a summary per person, and a filtered report.
- **Data pipelines** in analytics teams are chains of `filter` and `map` over typed records, just at a larger scale.
- **Code review.** Small named functions like `gradeFor` are what reviewers ask for when they see one 80-line function. They're easier to test and easier to trust.
