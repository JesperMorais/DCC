The office is voting on what to order for Friday lunch. Each vote is a string. Write `tally_votes(votes)` that returns a dict mapping each option to **how many votes it got**.

- Only options that got at least one vote appear in the result.
- An empty string is a **blank vote**. Don't count it.

Examples:

- `tally_votes(["pizza", "tacos", "pizza"])` → `{"pizza": 2, "tacos": 1}`
- `tally_votes(["sushi", "", "sushi"])` → `{"sushi": 2}`
- `tally_votes([])` → `{}`

The order of the keys doesn't matter.
