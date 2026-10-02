Every codebase has this helper somewhere:

```ts
function last(items: any[]): any {
  return items[items.length - 1];
}

const price = last(prices);   // any
price.toFixd(2);              // no error, until it runs 💥
```

It works for every array, which is why someone wrote it with `any`. But whatever goes in, `any` comes out, and from that line on the compiler has stopped checking. Typos, wrong methods and wrong arguments all compile. This lesson fixes `last` so that it still works for every array, but a `number[]` gives back a `number`.

### First: what does `any` actually do?

`any` doesn't mean "some type TypeScript will figure out later". It means **"stop checking"**. An `any` value can be passed anywhere, called like a function, and have any property read from it, and the compiler says yes to all of it. It also spreads: `price` is `any`, so `price.total` is `any`, and so is everything computed from it.

So `last(items: any[]): any` throws away the most useful fact you had, which is what kind of array the caller passed.

### Types disappear when the code runs

This is the step most people miss coming from other languages. TypeScript checks your code and then **erases** every type. The JavaScript that runs has none:

```ts
function last<T>(items: T[]): T { … }
// becomes, at runtime:
function last(items) { … }
```

So you can't ask "what is `T`?" inside the function. There's no `if (T === string)` and no `typeof T`, and `console.log(items)` only shows values. Generics aren't a runtime feature. They're a way of **describing, at the call site**, how the types going in relate to the types coming out.

### A type parameter is a placeholder

```ts
function last<T>(items: readonly T[]): T | undefined {
  return items[items.length - 1];
}
```

Read `<T>` as "for some type `T`, which the caller decides". The signature then says: you give me an array of `T`, and I give you back a `T` (or `undefined` if the array is empty). Each call fills in `T`:

```ts
last([9.5, 12, 3]);       // T = number  → number | undefined
last(["ada", "linus"]);   // T = string  → string | undefined
last(customers);          // T = Customer → Customer | undefined
```

You almost never write `last<number>(…)` yourself. TypeScript **infers** `T` from the arguments, the same way it infers `const x = 5` is a number. You only spell it out when there's nothing to infer from, like `new Set<string>()` or `useState<User | null>(null)`.

### A generic return type keeps the connection

The return type is where the value of a generic shows up. Compare:

```ts
function wrap(value: unknown): { value: unknown }    // caller learns nothing
function wrap<T>(value: T): { value: T }             // wrap(5).value is a number
```

`unknown` is the safe cousin of `any`: it accepts everything, but you can't *use* the result without checking it first. It's honest, but it still loses the type. `T` remembers it.

### More than one type parameter

When two inputs are unrelated, give each its own parameter:

```ts
function pair<A, B>(a: A, b: B): [A, B] {
  return [a, b];
}
pair("ada", 36); // [string, number]
```

If you used a single `T` for both, TypeScript would try to find *one* type for both arguments and report an error. One parameter means "these are the same type". Separate parameters mean "these can differ".

### What a bare `T` can't do

Inside `last`, `T` could be *anything*: a number, a string, `null`. So TypeScript won't let you read `item.id` or `item.name` from a bare `T`. That's correct, but it means you can't write `sortById<T>` or `findBy<T>` yet. To say "any type, *as long as it has an `id`*", you need a **constraint**. That's the next node.

### Gotchas

- **One `T` ties arguments together.** With `function push<T>(list: T[], item: T)`, `push([1, 2], "x")` is an error. That's usually what you want.
- **Don't add a type parameter that's used once.** `function log<T>(x: T): void` gains nothing over `x: unknown`. A generic earns its place when `T` appears at least twice, typically once in and once out.
- **`T[]` vs `readonly T[]`.** Take `readonly T[]` if you don't modify the array, so callers can pass either kind.
- **Name them simply.** `T`, `K`, `V` and `A`/`B` are conventional, or use a full word like `TItem` when there are several.

### In the wild

- **`Array<T>` itself** is generic, which is why `prices.map(...)`, `find` and `filter` all keep the element type.
- **`Promise<T>`**: `await fetchUser()` gives you a `User` because the function returns `Promise<User>`.
- **React's `useState<T>`** returns `[T, (value: T) => void]`, a generic tuple.
- **`Map<K, V>` and `Set<T>`** from the Core section were generics all along.
