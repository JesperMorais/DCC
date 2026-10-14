**Bug report from the club website:** "Top of the table looks right, but whenever two teams are level on everything, the order is backwards. Fans of Djurgården are emailing us because they're listed below Sirius."

`league_table(teams)` returns the standings as strings `"<position>. <name> <points>"`. A win is worth 3 points, a draw 1. Teams are ranked by:

1. **points**, highest first,
2. then **goal difference** (`goals_for - goals_against`), highest first,
3. then **name**, alphabetically (A before Z).

```python
league_table([
    TeamRecord("Hammarby", won=1, drawn=1, lost=2, goals_for=5, goals_against=6),
    TeamRecord("Malmö",    won=3, drawn=0, lost=1, goals_for=9, goals_against=3),
])
# ["1. Malmö 9", "2. Hammarby 4"]

# Level on points: better goal difference first
# Häcken 6 pts (+3), Elfsborg 6 pts (0)  →  ["1. Häcken 6", "2. Elfsborg 6"]
```

**Reproduction:** three teams, each with 1 win, 2 draws, 1 loss and a goal difference of 0: Sirius, Djurgården and Mjällby. The function returns `["1. Sirius 5", "2. Mjällby 5", "3. Djurgården 5"]`, expected `["1. Djurgården 5", "2. Mjällby 5", "3. Sirius 5"]`.
