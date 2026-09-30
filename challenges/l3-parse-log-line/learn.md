### From raw text to a typed object

Parsing turns an untyped `string` into something with a real shape. A common pattern is: **cut** the string, **validate** the pieces, then **build** the object.

```ts
interface Version { major: number; minor: number }

function parseVersion(s: string): Version | null {
  const m = s.match(/^(\d+)\.(\d+)$/);   // cut
  if (!m) return null;                    // validate
  return { major: Number(m[1]), minor: Number(m[2]) }; // build
}
```

`match` with capture groups `( … )` returns an array: index 0 is the whole match, then one entry per group — or `null` if it didn't match.

### Literal unions need proof

```ts
type Size = "S" | "M" | "L";
const raw: string = "M";
const size: Size = raw;             // Error: string is not Size
if (raw === "S" || raw === "M" || raw === "L") {
  const ok: Size = raw;             // fine — narrowed by the checks
}
```

A plain `string` could be anything, so TypeScript only accepts it as a `Size` after you've checked.

### A filter that narrows

`array.filter((x): x is Foo => x !== null)` tells TypeScript the result contains only `Foo`s.
