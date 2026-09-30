Your shop's frontend keeps the cart in a **reducer**: a pure function that takes the current state and an action, and returns the **next** state. Implement `cartReducer(state, action)` for the four actions in `CartAction`:

| Action | Effect |
|---|---|
| `{ type: "add", line }` | If a line with that `id` is already in the cart, increase its `qty` by 1. Otherwise append it with `qty: 1`. |
| `{ type: "remove", id }` | Remove the line with that `id` (no line → nothing changes). |
| `{ type: "setQty", id, qty }` | Set that line's `qty`. If `qty` is `0` or less, remove the line instead. |
| `{ type: "clear" }` | Empty the cart. |

Rules:

- **Never mutate** `state` or its lines — always return new objects/arrays. (Undo and change-detection rely on this.)
- Handle the actions with a `switch` on `action.type` that is **exhaustive**: if someone adds a fifth action type later, your reducer should fail to compile until it handles it (use `never`).

```ts
let cart: CartState = { lines: [] };
cart = cartReducer(cart, { type: "add", line: { id: "tea", name: "Tea", priceCents: 300 } });
// { lines: [{ id: "tea", name: "Tea", priceCents: 300, qty: 1 }] }
cart = cartReducer(cart, { type: "add", line: { id: "tea", name: "Tea", priceCents: 300 } });
// tea now has qty 2
cart = cartReducer(cart, { type: "setQty", id: "tea", qty: 0 });
// { lines: [] }
```
