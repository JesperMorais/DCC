A coding-quiz site shows a leaderboard after each round. Every result is a tuple `(name, score, seconds)`, where `seconds` is how long the player took. Write `leaderboard(results)` that returns the ranked lines.

Ranking rules, in order:

1. **Higher score** first.
2. Same score: **faster** player (fewer seconds) first.
3. Still tied: **name** alphabetically.

Each line is `"<rank>. <name> (<score> pts)"`, with ranks starting at **1**. Don't reorder the list you were given. No results gives `[]`.

```python
leaderboard([("ada", 80, 42.0), ("linus", 95, 60.5), ("grace", 80, 39.9)])
# ["1. linus (95 pts)", "2. grace (80 pts)", "3. ada (80 pts)"]
```
