In 2009, Tony Hoare, who invented the null reference in 1965, called it his "billion-dollar mistake": decades of crashes from code that used a value that wasn't there. JavaScript's version is the most common error in the language, `Cannot read properties of null`.

TypeScript's answer is two ideas that work together. **Union types** say "this value is one of these things". **Narrowing** lets the compiler follow your `if`s and know which one you have.

### Union types: "one of these"

A **union** joins types with `|`. A value of the union can be any one of its members:

```ts
type OrderId = number | string; // 1042 or "A-1042"
let next: string | null = null; // a string, or nothing yet
```

You've already used the special case: a **literal union** like `"small" | "medium" | "large"` is a union of three single-value types.

Here's the catch. Before you check, TypeScript only lets you do what's valid for **every** member:

```ts
function show(id: OrderId) {
  return id.toUpperCase();
  // ✗ Property 'toUpperCase' does not exist on type 'number'.
}
```

It's right: if `id` is `1042`, there is no `toUpperCase`. You need to find out which one you have.

### Narrowing with `typeof`

Types are erased when the code runs, so you can't ask "is this an `OrderId`?". You *can* ask JavaScript what kind of value it is. `typeof x` returns a string at runtime: `"string"`, `"number"`, `"boolean"`, `"undefined"`, `"object"`, `"function"` (plus two rare ones, `"bigint"` and `"symbol"`).

TypeScript reads those checks and **narrows** the type inside each branch:

```ts
function show(id: OrderId): string {
  if (typeof id === "number") {
    return `#${id}`;            // here id: number
  }
  return id.toUpperCase();      // here id: string
}
```

Notice the second line has no `else`. TypeScript follows the **control flow**: the `number` case returned, so whatever reaches the last line must be a `string`. Hover over `id` in your editor and you'll see the type change from line to line.

`null` works the same way, with `===`:

```ts
function greet(name: string | null): string {
  if (name === null) return "Hello, guest";
  return `Hello, ${name.trim()}`; // name: string
}
```

### Narrowing literal unions with `===`

Comparing against a literal narrows a literal union, and a `switch` does it case by case:

```ts
type Size = "small" | "medium" | "large";

function ml(size: Size): number {
  switch (size) {
    case "small":  return 250;
    case "medium": return 350;
    case "large":  return 450;
  }
}
```

TypeScript knows the three cases cover every `Size`, so it doesn't complain about a missing return. Add `"xl"` to `Size` and this function becomes a compile error until you handle it.

### Narrowing object shapes with `in`

`typeof` says `"object"` for every object, so it can't tell two shapes apart. The `in` operator checks whether a property **exists** on the value at runtime, and TypeScript narrows on it:

```ts
type Email = { email: string };
type Phone = { phone: string; ext?: string };

function contact(c: Email | Phone): string {
  if ("email" in c) return `mailto:${c.email}`; // c: Email
  return `tel:${c.phone}`;                       // c: Phone
}
```

Pick a property that only one member has. Properties shared by every member, you can read without narrowing at all.

### Worked example: a flexible price

A shop's import file has prices as numbers, as strings like `"19.90"`, or missing (`null`):

```ts
type RawPrice = number | string | null;

function toCents(p: RawPrice): number {
  if (p === null) return 0;
  if (typeof p === "string") p = Number(p); // p: string, then reassigned to a number
  return Math.round(p * 100);               // p: number
}
```

Each check removes one possibility, until only `number` is left. This is the everyday shape of narrowing: handle the odd cases first, return or convert, and the main path has one clean type.

### Gotchas

- **`typeof null` is `"object"`.** It's a 30-year-old JavaScript bug that can't be fixed. Check `x === null` explicitly.
- **There's no `typeof x === "array"`.** Arrays are `"object"` too. Use `Array.isArray(x)`.
- **Truthiness narrows, but it also drops `0` and `""`.** `if (count) …` removes `null` *and* a real `0`. When `0` is a valid value, compare with `!== null` or `!== undefined`.
- **`in` is a runtime check.** It looks at the actual object, so it's only safe when the property name really identifies the shape.
- **`any` turns narrowing off.** If a value is `any`, every check "succeeds" and nothing is protected. Give it a union type instead.

### In the wild

- **DOM APIs** return unions: `document.querySelector("#app")` is `Element | null`, so every UI codebase starts with `if (el === null) …`.
- **Express and Node** type query parameters as `string | string[] | undefined`, so handlers narrow with `typeof` and `Array.isArray`.
- **Redux Toolkit and React Query** use literal unions such as `status: "idle" | "loading" | "succeeded" | "failed"`, narrowed in components with `if (status === "loading")`.
- **API clients** distinguish response shapes with `in`, like `if ("error" in res)` before reading `res.data`.
