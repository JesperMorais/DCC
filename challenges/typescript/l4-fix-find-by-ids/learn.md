### Type guards

A function whose return type is `value is T` is a **type guard**. When it returns `true`, TypeScript narrows the value to `T`:

```ts
function isString(x: unknown): x is string {
  return typeof x === "string";
}

const items: unknown[] = ["a", 1, "b"];
const strings = items.filter(isString); // string[]
```

Here's the catch: TypeScript **trusts** the guard. It never checks that the body really proves what the return type claims. A guard that lies compiles fine, and the wrong values flow into code that thinks they can't be there.

### Two kinds of "nothing"

JavaScript has both `null` (usually "deliberately empty") and `undefined` (usually "not there", e.g. a missing property or `map.get` of an unknown key). They are not `===` to each other. The loose `x == null` is true for both; `x === null` only for `null`.

### Debugging technique: check the data where it's created

When a crash says "undefined", don't start at the crash. Find where the value was *produced* and log it there. If it's already wrong at the source, the bug is upstream of the crash.
