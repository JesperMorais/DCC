A food-delivery dashboard had a "Cancelled orders" tab that was always empty. The code filtered on `order.status === "canceled"`, and the backend sent `"cancelled"` with two l's. Both are perfectly good strings, so nothing complained. Customers kept getting charged for orders nobody saw.

This lesson is about describing the *shape* of your data, so a typo like that becomes a red squiggle instead of a support ticket.

### Objects bundle named values

So far you've had single values: a number, a string. An **object** groups several of them under names, called **properties**:

```ts
const order = { id: 1042, customer: "Ada", total: 129, status: "placed" };

order.customer; // "Ada"   read a property with a dot
order.total;    // 129
```

If you've used Python dicts, Java classes or C structs, this is the everyday version of those. In JavaScript you don't need a class to make one; the `{ … }` literal *is* the object.

### Type aliases: give a shape a name

TypeScript already infers a type for `order` above. To reuse that shape in function parameters, give it a name with `type`:

```ts
type Order = {
  id: number;
  customer: string;
  total: number;
  status: string;
};

function isBig(order: Order): boolean {
  return order.total > 500;
}
```

`type Order = …` is a **type alias**. It creates no value and no code. It's a name for a type, the same way a variable is a name for a value. An **object type** lists each property with its type, separated by `;`.

Now the compiler checks every object that claims to be an `Order`:

```ts
isBig({ id: 1, customer: "Ada", total: 80 });
// ✗ Property 'status' is missing

isBig({ id: 1, customer: "Ada", total: "80", status: "placed" });
// ✗ Type 'string' is not assignable to type 'number'

order.custmer;
// ✗ Property 'custmer' does not exist. Did you mean 'customer'?
```

You also get autocomplete: type `order.` and the editor lists the four properties.

### Literal union types: "one of these exact strings"

`status: string` still allows `"canceled"`, `"cancelled"` and `"banana"`. You want *exactly* the statuses your backend uses. A string literal like `"placed"` can itself be a type: the type whose only value is that string. Join a few with `|` and you get a **literal union**:

```ts
type Status = "placed" | "cooking" | "delivered" | "cancelled";

type Order = {
  id: number;
  customer: string;
  total: number;
  status: Status;
};

order.status === "canceled";
// ✗ This comparison appears to be unintentional because the types
//   'Status' and '"canceled"' have no overlap.
```

That's the dashboard bug from the opening, caught as you type. Read `|` as "or": a `Status` is `"placed"` *or* `"cooking"` *or* …

### Worked example: a status update

Say you need a function that marks an order as delivered. You can't change the order you were given, because other parts of the app hold the same object. Build a new one instead:

```ts
function markDelivered(order: Order): Order {
  return {
    id: order.id,
    customer: order.customer,
    total: order.total,
    status: "delivered",
  };
}

const before: Order = { id: 7, customer: "Linus", total: 95, status: "cooking" };
const after = markDelivered(before);

before.status; // "cooking"   untouched
after.status;  // "delivered"
```

Writing every field out gets tedious. Core TypeScript shows you a one-line shortcut (object spread). The idea is the same: a *new* object.

### Gotchas

- **`const` doesn't freeze an object.** `const order = {…}` means the name `order` can't point at a different object. You can still write `order.total = 0`, and everyone holding that object sees the change.
- **`let` widens literals.** `let s = "placed"` is inferred as `string`, because you might reassign it. `const s = "placed"` is the literal `"placed"`. If you build a value with `let` and pass it as a `Status`, annotate it: `let s: Status = "placed"`.
- **Types are erased.** `console.log(order)` shows the values `{ id: 7, … }`, never `Order`. The type only exists while you write code, so you can't check it at runtime with something like `if (x is Order)`.
- **`string`, not `String`.** Use lowercase `string`, `number` and `boolean` in object types. The capitalised versions are wrapper objects you almost never want.
- **Extra properties in a literal are an error.** Passing `{ id: 1, …, status: "placed", tip: 5 }` straight into `isBig` fails, because `tip` isn't part of `Order`. That catches typos in property names too.

### In the wild

- **API responses.** Front-end teams write `type User = { id: number; name: string; … }` for what `/api/me` returns, so every screen that reads `user.nmae` fails to compile.
- **Redux and React state** almost always use literal unions for status: `status: "idle" | "loading" | "failed"`.
- **Stripe's and GitHub's TypeScript SDKs** type fields like `status: "active" | "past_due" | "canceled"`. The spelling bug from the opening can't get past the compiler.
- **Config files.** Tools like ESLint and Vite type their options as object types with literal unions, which is why your editor can autocomplete `"error" | "warn" | "off"`.
