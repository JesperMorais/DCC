### Conditional types

Types can branch. `T extends U ? X : Y` reads "if `T` is assignable to `U`, the result is `X`, otherwise `Y`":

```ts
type IsString<T> = T extends string ? "yes" : "no";
type A = IsString<"hi">;  // "yes"
type B = IsString<42>;    // "no"
```

### `infer`: pattern matching for types

Inside the `extends` clause you can leave a *hole* and let TypeScript fill it in. `infer X` declares a new type variable, bound to whatever sits in that position:

```ts
type ElementOf<T> = T extends (infer E)[] ? E : never;
type N = ElementOf<number[]>;          // number

type FirstArg<F> = F extends (first: infer A, ...rest: any[]) => any ? A : never;
type S = FirstArg<(s: string, n: number) => void>;  // string
```

`infer` only works inside the `extends` of a conditional type, and the inferred variable is only in scope in the **true** branch.

### Constraints vs. conditions

`MyType<T extends Foo>` *rejects* bad input at the call site (a compile error). `T extends Foo ? … : never` inside the body *tolerates* it and yields `never`. You often want both: a constraint for good errors, a conditional for the extraction.
