### Utility types transform other types

TypeScript ships generic helper types that build a new type from an existing one, so you don't have to repeat yourself:

```ts
interface Post { id: number; title: string; body: string; draft: boolean }

type PostDraft   = Partial<Post>;             // every property optional
type PostPreview = Pick<Post, "id" | "title">; // only these keys
type NewPost     = Omit<Post, "id">;          // everything except id
type Frozen      = Readonly<Post>;            // every property readonly
```

They compose — the output of one is just a type you can feed into another:

```ts
type EditableFlags = Partial<Pick<Post, "draft">>; // { draft?: boolean }
```

When `Post` gets a new field later, every derived type updates automatically.

### Spread copies, later wins

```ts
const base = { a: 1, b: 2 };
const next = { ...base, b: 3 }; // { a: 1, b: 3 } — base is untouched
```

Spread makes a **shallow copy**, so it's the standard way to "update" an object without mutating the original.
