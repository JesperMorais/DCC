**Bug report from the charity auction:** "Someone bid 120 on the painting and someone else bid 9. The screen announced **9** as the winning bid."

Bids come in from a web form as text. `highest_bid(bids)` returns the highest bid as an `int`.

- Surrounding spaces are ignored (`" 45 "` is a bid of 45).
- Empty or all-space entries are not bids and are skipped.
- If there are no bids at all, return `0`.

Examples:

- `highest_bid(["15", " 40", "25 "])` → `40`
- `highest_bid(["", "  "])` → `0`
- `highest_bid(["300", "1000"])` → `1000`

**Reproduction:** `highest_bid(["9", "120"])` returns `9`, expected `120`.
