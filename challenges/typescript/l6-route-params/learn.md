### Template literal types can be *matched*

You've seen template literal types *build* strings. Combined with `infer` inside a conditional type they also *take strings apart*:

```ts
type Domain<E> = E extends `${string}@${infer D}` ? D : never;
type D = Domain<"ada@example.com">;   // "example.com"

type Split<S> = S extends `${infer Head},${infer Tail}` ? [Head, Tail] : [S];
type P = Split<"a,b,c">;   // ["a", "b,c"]  — the first infer grabs as little as possible
```

`${string}` is a wildcard that matches any text (including empty) without naming it.

### Recursion over strings

To handle "any number of" pieces, recurse on what's left and union the results:

```ts
type Words<S> = S extends `${infer W} ${infer Rest}` ? W | Words<Rest> : S;
type W = Words<"red green blue">;   // "red" | "green" | "blue"
```

Always ask: what happens on the **last** piece (no separator after it), and on the **empty** string?

### From a union of keys to an object

Once you have a union of names, a mapped type `{ [K in Names]: ... }` turns it into an object. Mapping over `never` gives `{}`.
