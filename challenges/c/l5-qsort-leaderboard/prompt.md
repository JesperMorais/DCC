A game server keeps its high scores in an array of structs. Sort it for display using the standard library's `qsort`:

```c
typedef struct {
    char name[16];
    int score;
} Player;

void sort_leaderboard(Player *players, size_t count);
```

- **Highest score first.**
- Players with the same score are ordered by name, **alphabetically** (plain `strcmp` order).
- Scores can be anything an `int` can hold, including negative scores, `INT_MIN` and `INT_MAX`.
- `count` may be 0 or 1.

```c
Player board[] = {{"ada", 50}, {"bob", 90}, {"cy", 50}};
sort_leaderboard(board, 3);
// → bob 90, ada 50, cy 50
```

Use `qsort` from `<stdlib.h>` with your own comparator function. Don't write the sort yourself.
