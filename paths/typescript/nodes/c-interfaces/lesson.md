A video player let you mute it by dragging the volume to 0. Reload the page, and it came back at 50%. Users filed it as "mute doesn't save". The settings code was one line:

```ts
const volume = saved.volume || 50;
```

`0 || 50` is `50`. This lesson covers how to describe data where some fields may be missing, and how to fill the gaps without that bug.

### Interfaces: another way to name a shape

In Fundamentals you named object shapes with `type`. An **interface** does the same job with slightly different syntax:

```ts
interface Track {
  id: number;
  title: string;
  seconds: number;
}
```

No `=`, and the body is just the property list. Everything you know still applies: a missing property, a wrong type or a misspelled name is a compile error.

An interface can **extend** another one, which means "everything that one has, plus these":

```ts
interface Podcast extends Track {
  host: string;
}
// Podcast has id, title, seconds and host
```

A `Podcast` can be passed anywhere a `Track` is expected, because it has everything a `Track` needs.

### Interface or type alias?

For plain object shapes they're interchangeable, and you'll see both in real code. The differences:

| | `interface` | `type` |
|---|---|---|
| object shapes | ✓ | ✓ |
| unions like `"a" \| "b"` | ✗ | ✓ |
| extending | `extends` | `&` (intersection) |
| can be declared twice and merged | ✓ | ✗ (duplicate name error) |

A common convention: `interface` for object shapes, `type` for unions and everything else. Whichever your team picks, be consistent.

### Optional properties: `?`

Real data has gaps. A track might have a `rating`, or might not. Mark that with `?`:

```ts
interface Track {
  id: number;
  title: string;
  rating?: number; // may be missing
}

const a: Track = { id: 1, title: "Intro" };             // ✓ no rating
const b: Track = { id: 2, title: "Outro", rating: 4 };  // ✓
```

Reading an optional property gives you `number | undefined`, so TypeScript makes you deal with the missing case:

```ts
a.rating * 2;
// ✗ 'a.rating' is possibly 'undefined'.
```

This is the missing step people skip: `?` doesn't mean "TypeScript fills in a default". At runtime the property simply isn't there, and reading it gives `undefined`.

### `??`: a default only for *missing* values

`||` falls back whenever the left side is **falsy**: `undefined`, `null`, but also `0`, `""` and `false`. That's the mute bug. The **nullish coalescing** operator `??` only falls back on `undefined` or `null`:

```ts
0 || 50;          // 50   ✗ the user's 0 is lost
0 ?? 50;          // 0    ✓
undefined ?? 50;  // 50   ✓ missing, so use the default
```

Rule of thumb: when a field is optional and `0`, `""` or `false` are real answers, use `??`.

### Object spread: copy, then change

`{ ...obj }` copies every property of `obj` into a new object. Write more properties after it to override them. **Later wins:**

```ts
const t: Track = { id: 3, title: "Demo", rating: 2 };

const renamed = { ...t, title: "Demo (final)" };
// { id: 3, title: "Demo (final)", rating: 2 }    t is unchanged
```

This is the one-line version of "build a new object" from Fundamentals.

### Worked example: player settings

```ts
interface PlayerSettings {
  volume?: number;
  shuffle?: boolean;
}

function effectiveVolume(s: PlayerSettings): number {
  return s.volume ?? 50;
}

function mute(s: PlayerSettings): PlayerSettings {
  return { ...s, volume: 0 };
}

effectiveVolume({});                      // 50
effectiveVolume(mute({ shuffle: true })); // 0, and it stays 0 after a reload
```

`effectiveVolume` handles the gap with `??`. `mute` copies whatever settings exist and overrides one field, without touching the caller's object.

### Gotchas

- **`||` vs `??`.** `||` also replaces `0`, `""` and `false`. Use `??` for "missing".
- **Spread is shallow.** `{ ...playlist }` copies the `tracks` *reference*, not the array. Pushing to `copy.tracks` changes the original's tracks too. Copy nested arrays explicitly: `{ ...p, tracks: [...p.tracks, t] }`.
- **Order matters.** `{ volume: 0, ...s }` puts the override *first*, so `s.volume` wins. The override goes last.
- **Explicit `undefined` overwrites.** `{ ...defaults, ...{ volume: undefined } }` has `volume: undefined`, because the key exists. That's another reason to read with `??` at the point of use.
- **Optional is not the same as `| undefined`.** `rating?: number` lets you leave the key out. `rating: number | undefined` makes you write `rating: undefined` explicitly.

### In the wild

- **React props** are interfaces with optional fields: `interface ButtonProps { label: string; disabled?: boolean }`, read with `disabled ?? false`.
- **Redux reducers** return `{ ...state, loading: true }` on every action, never mutating the old state.
- **API types** for `PATCH` requests make every field optional, because the client only sends what changed.
- **Config files** like `vite.config.ts` are interfaces with dozens of optional fields, each with a default applied via `??` inside the tool.
