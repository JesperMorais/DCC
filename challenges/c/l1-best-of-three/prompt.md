In a quiz game, every player plays three rounds, and only their **best** round counts for the leaderboard. Rounds can go badly: wrong answers cost points, so a score can be **negative**.

Write `max_of_three`, which returns the largest of three scores:

```c
int max_of_three(int a, int b, int c);
```

Examples:

- `max_of_three(12, 40, 7)` → `40`
- `max_of_three(5, 5, 5)` → `5`
- `max_of_three(-8, -3, -20)` → `-3` (the least-bad round)

Don't use any library functions. Plain `if` statements are all you need.
