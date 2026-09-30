### Optional properties

A `?` after a property name means "this may be missing":

```ts
interface Profile {
  name: string;
  bio?: string; // type is string | undefined
}
```

Because `bio` might be `undefined`, TypeScript won't let you treat it as a plain `string` until you handle the missing case.

### `??` — the nullish coalescing operator

`a ?? b` gives `a`, unless `a` is `null` or `undefined` — then it gives `b`.

```ts
const bio = profile.bio ?? "No bio yet";
```

Compare it with `||`, which falls back on **every** falsy value: `""`, `0`, `NaN` and `false`.

```ts
const count = 0;
count || 10; // 10  — probably a bug!
count ?? 10; // 0   — kept, because 0 is a real value
```

Rule of thumb: use `??` for "use a default if it's not set".
