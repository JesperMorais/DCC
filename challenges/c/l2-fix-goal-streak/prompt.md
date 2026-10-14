**Bug report from a step-counter app user:** "I hit my step goal every single day for the last five days, and my profile still says my best streak is 2 days. That was from two weeks ago!"

`longest_goal_streak` returns the length of the longest run of **consecutive** days where the step goal was reached:

```c
#include <stddef.h>

size_t longest_goal_streak(const int *steps, size_t count, int goal);
```

- A day reaches the goal if its steps are **greater than or equal to** `goal`.
- A day below the goal breaks the streak.
- No days, or no day reaching the goal, gives `0`.

What it should do (goal `8000`):

```c
int week1[] = {9000, 8000, 3000, 8500, 12000, 9100, 2000};
int week2[] = {5000, 4000, 7999};
longest_goal_streak(week1, 7, 8000);   // → 3 (8500, 12000, 9100)
longest_goal_streak(week2, 3, 8000);   // → 0
```

**Reproduction:**

```c
int recent[] = {9000, 9500, 1200, 8000, 8800, 10000, 9100, 8300};
longest_goal_streak(recent, 8, 8000);  // returns 2, expected 5
```

Find the bug and fix it.
