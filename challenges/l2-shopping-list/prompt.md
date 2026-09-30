Write three helpers for a shopping list (an array of strings).

**`hasItem(list, item)`** — is the item on the list?

- `hasItem(["milk", "eggs"], "eggs")` → `true`
- `hasItem(["milk", "eggs"], "bread")` → `false`

**`positionOf(list, item)`** — at which index is the item? Return `-1` if it isn't there.

- `positionOf(["milk", "eggs", "jam"], "jam")` → `2`
- `positionOf(["milk"], "tea")` → `-1`

**`addIfMissing(list, item)`** — return a list with the item added **at the end**, but only if it isn't already there.
Don't change the original array — return a new one.

- `addIfMissing(["milk"], "eggs")` → `["milk", "eggs"]`
- `addIfMissing(["milk", "eggs"], "milk")` → `["milk", "eggs"]`

Matching is exact (`"Milk"` and `"milk"` are different items).
