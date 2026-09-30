Two players play **rock, paper, scissors**. The rules:

- rock beats scissors
- scissors beats paper
- paper beats rock
- the same move on both sides is a draw

**Step 1 — the type.** The starter says `type Move = string;`, which allows nonsense like `"lizard"`.
Change `Move` so that **only** `"rock"`, `"paper"` and `"scissors"` are allowed.
The tests check this: a line that tries to use `"lizard"` must become a type error.

**Step 2 — the function.** Write `playRound(p1, p2)` that returns who won, as a `Result`:

- `playRound("rock", "scissors")` → `"player1"`
- `playRound("rock", "paper")` → `"player2"`
- `playRound("paper", "paper")` → `"draw"`
