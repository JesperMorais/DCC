A food-delivery app adds a new order state, `"refunded"`, on the backend. The release goes out on Friday. On Saturday, refunded orders show up in the app as a blank grey card. Nobody updated the function that turns an order into a label, and nothing complained:

```ts
function label(order: Order): string {
  if (order.status === "placed") return "Order placed";
  if (order.status === "cooking") return "In the kitchen";
  if (order.status === "on_the_way") return "On the way";
  return ""; // refunded ends up here, silently
}
```

This lesson shows how to model "one of several shapes" so that TypeScript knows which fields each shape has, and so that forgetting a case is a **compile error**, not a blank card.

### First: a union of object shapes

In *Unions & narrowing* you used unions like `string | number`. A union can just as well be made of **object types**:

```ts
type Payment =
  | { method: "card"; last4: string }
  | { method: "swish"; phone: string }
  | { method: "invoice"; email: string };
```

Read it as "a `Payment` is exactly one of these three shapes". A card payment has `last4` and no `phone`. An invoice has `email` and nothing else.

The field `method` is special. Every shape has it, and in each shape it's a different **literal type**: `"card"`, `"swish"`, `"invoice"`, not just `string`. That shared literal field is called the **tag** (or *discriminant*), and the whole thing is a **discriminated union**.

### Checking the tag narrows the whole object

Before you check anything, TypeScript only lets you touch fields that *every* shape has:

```ts
function show(p: Payment) {
  p.method; // ✓ every shape has it
  p.last4;  // ✗ Property 'last4' does not exist on type '{ method: "swish"; … }'
}
```

Check the tag, and TypeScript narrows `p` to the one shape that matches:

```ts
function show(p: Payment): string {
  switch (p.method) {
    case "card":
      return `Card ending in ${p.last4}`; // p is the card shape here
    case "swish":
      return `Swish to ${p.phone}`;
    case "invoice":
      return `Invoice to ${p.email}`;
  }
}
```

Inside `case "card"`, TypeScript knows `p.method` is `"card"`, so `p` must be the card shape, so `p.last4` is a `string`. No casts, no `?.`.

Remember that types are erased at runtime. The tag is a real field that exists in the JavaScript object, which is exactly why it works: the code checks a **value**, and TypeScript follows along.

### `never`: the type with no values

`string` has many values. `"card"` has one. `never` has **zero**. No value can ever have type `never`.

That sounds useless, but it's what you get when narrowing has removed every option. After the three cases above, nothing is left:

```ts
switch (p.method) {
  case "card": …
  case "swish": …
  case "invoice": …
  default:
    p; // type: never — no shape is left
}
```

So write a helper that only accepts `never`:

```ts
function assertNever(value: never): never {
  throw new Error(`Unexpected value: ${JSON.stringify(value)}`);
}
```

Its return type is also `never`, because it never returns: it always throws. Now put it in the `default`:

```ts
default:
  return assertNever(p);
```

Today this compiles, because `p` really is `never` there. Next month someone adds `{ method: "klarna"; … }` to `Payment`. Now `p` in the `default` is the klarna shape, which isn't assignable to `never`, and the compiler points at the exact line:

```
Argument of type '{ method: "klarna"; … }' is not assignable to parameter of type 'never'.
```

That's an **exhaustive check**. The blank grey card from the opening becomes a red squiggle on the day `"refunded"` is added.

### Why it still throws at runtime

If the compiler already guarantees `default` is unreachable, why does `assertNever` throw? Because types only describe what you *told* TypeScript. Data from an API, a database or an old app version can contain a tag your types don't know about. The `throw` turns "silently wrong" into a loud, debuggable error with the bad value in the message.

### Gotchas

- **The tag must be a literal type.** If you write `method: string` in a shape, checking `p.method === "card"` narrows nothing, because every shape could have `"card"`.
- **A `default: return ""` hides missing cases.** It compiles no matter how many shapes you add. Use `assertNever` instead.
- **Don't modify the input to change state.** To go from "cooking" to "on the way", build a new object: `{ status: "on_the_way", driver }`. Mutating `order.status` leaves the old shape's fields behind.
- **`in` works, but the tag is better.** `"last4" in p` narrows too, but it breaks if two shapes share a field. A tag is explicit and survives refactors.

### In the wild

- **Redux and `useReducer`** actions are discriminated unions on `type`: `{ type: "add"; item } | { type: "clear" }`, handled with a `switch`.
- **TypeScript's own compiler** represents every syntax node with a `kind` field and switches on it everywhere.
- **Result types** in libraries like `neverthrow` are `{ ok: true; value } | { ok: false; error }`, which is a union tagged by a boolean literal.
- **Zod** has `z.discriminatedUnion("type", [...])` to validate API data into exactly this shape, so the runtime check and the type agree.
