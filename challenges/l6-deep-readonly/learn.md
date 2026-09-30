### Recursive type aliases

A type can refer to itself, just like a recursive function. The classic example is JSON:

```ts
type Json = string | number | boolean | null | Json[] | { [key: string]: Json };
```

When you write a recursive **transformation**, the structure mirrors a recursive function: a mapped type does the "loop over properties", and calling the type again on each value is the "recursive call":

```ts
type DeepNullable<T> = {
  [K in keyof T]: DeepNullable<T[K]> | null;
};
```

### Base cases matter

Without a base case the mapped type is applied to *everything*, including things it shouldn't touch. Mapping over a function type, for instance, keeps only its (usually zero) properties and loses the call signature — the result is no longer callable. So recursive types usually start with a conditional that decides *whether* to recurse:

```ts
type Wrap<T> = T extends string ? T : { inner: Wrap<T> };  // shape only
```

### Arrays for free

A **homomorphic** mapped type (`[K in keyof T]` where `T` is a type parameter) is smart about arrays and tuples: instantiated with `[1, 2]` it produces a *tuple*, not an object with keys `"0"`, `"1"`, `"length"`… — and `readonly` on it yields a `readonly` tuple.
