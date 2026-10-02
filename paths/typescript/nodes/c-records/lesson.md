A team counted page views with a plain object, `counts[path] = (counts[path] ?? 0) + 1`, and it worked for a year. Then a crawler requested a page called `constructor`, and the count came out as `"function Object() { [native code] }1"`. A plain `{}` already has keys you never put there, inherited from `Object.prototype`. This lesson covers the three ways to keep "a value per key" in TypeScript, `Record`, `Map` and `Set`, and how to pull keys out of text in the first place.

### Objects as maps, typed with `Record`

A plain object is the quickest lookup table. `Record<K, V>` is the type for "an object whose keys are `K` and whose values are `V`":

```ts
const views: Record<string, number> = {};
views["/pricing"] = (views["/pricing"] ?? 0) + 1;
```

`Record` really shines when the keys are a **known, finite set**, such as a literal union:

```ts
type Level = "info" | "warn" | "error";
const perLevel: Record<Level, number> = { info: 0, warn: 0, error: 0 };
// leave one out → ✗ Property 'error' is missing
```

Now every key must be present, and a typo like `perLevel.eror` is an error. That's a guarantee you don't get from a `Map`.

Two things to know about `Record<string, V>`. First, reading a missing key gives `undefined` at runtime, but the type says `V` (unless `noUncheckedIndexedAccess` is on), so use `?? 0` when counting. Second, object keys are always **strings**: `obj[1]` and `obj["1"]` are the same slot.

### `Map`: a real dictionary

`Map<K, V>` is built for keys that come from data:

```ts
const lastSeen = new Map<string, string>();
lastSeen.set("ada", "09:15");
lastSeen.get("ada");      // "09:15"  (type: string | undefined)
lastSeen.has("linus");    // false
lastSeen.size;            // 1
for (const [user, time] of lastSeen) { … } // insertion order
```

Compared with an object, a `Map` has no inherited keys (so no `constructor` surprise), keys can be any type (numbers stay numbers, and objects work too), it has a real `.size`, and `get` honestly returns `V | undefined`.

**Rule of thumb:** use a `Record` for a fixed set of keys you know while writing the code, or for something you'll send as JSON. Use a `Map` for keys that come from the data.

### `Set`: "have I seen this before?"

A `Set` holds each value **once**. Adding a value that's already there does nothing:

```ts
const visitors = new Set<string>();
visitors.add("ada");
visitors.add("linus");
visitors.add("ada");
visitors.size;          // 2
visitors.has("grace");  // false
[...visitors];          // ["ada", "linus"], in first-seen order
```

`has` is fast no matter how big the set gets, which `array.includes` isn't. Sets compare with `===`, so two different objects with the same fields count as two values. Strings and numbers work the way you'd hope.

Sets and maps combine nicely. "Unique visitors per page" is a `Map<string, Set<string>>`.

### Pulling fields out of a line: regex

Log lines are text, and you need the key out of them first. A **regular expression** describes the shape of the line, and parentheses **capture** the parts you want:

```ts
const line = "09:15 ada /pricing";
const m = line.match(/^(\d\d:\d\d) ([a-z]+) (\/\S*)$/);
if (m) {
  const [, time, user, path] = m; // m[0] is the whole match
}
```

Reading that pattern: `^` and `$` anchor it to the whole line, `\d` is a digit, `[a-z]+` is one or more lowercase letters, `\S*` is any run of non-space characters, and `\/` is a literal `/`. `match` returns `null` when the line doesn't fit, so **check before you destructure**. That `if (m)` is narrowing, like in Unions & narrowing.

For simpler formats, `split` is often enough: `"a,b,c".split(",")` gives `["a", "b", "c"]`. For text with several lines, use `text.split("\n")`, and `trim()` each line before matching, because files often end with a newline or contain `\r`.

### Worked example: who's talking in the chat?

Chat lines look like `"ada: hello"`. Count messages per user, and list the users in the order they first spoke:

```ts
function chatStats(lines: readonly string[]) {
  const messages = new Map<string, number>();
  const seen = new Set<string>();
  const newcomers: string[] = [];
  for (const line of lines) {
    const m = line.trim().match(/^([a-z]+): (.+)$/);
    if (!m) continue;                 // skip junk lines
    const [, user] = m;
    if (!seen.has(user)) {            // first time we see this user
      seen.add(user);
      newcomers.push(user);
    }
    messages.set(user, (messages.get(user) ?? 0) + 1);
  }
  return { messages, newcomers };
}
```

The regex finds the key, the `Set` answers "first time?", and the `Map` keeps a count per key. You'll see that combination again and again.

### Gotchas

- **`for...in` on an object gives keys, `for...of` on a Map gives `[key, value]` pairs.** Use `Object.entries(obj)` to get pairs from an object.
- **`JSON.stringify(new Map(...))` is `"{}"`.** Convert first with `Object.fromEntries(map)`.
- **A regex without `^…$` matches anywhere**, so `"xx 09:15 ada /a yy"` would sneak through.
- **`String` ≠ `string`.** Write `Map<string, number>`, not `Map<String, Number>`.

### In the wild

- **Analytics tools** count events per page and dedupe visitors with exactly this map-of-sets shape.
- **HTTP headers, environment variables (`process.env`) and JSON config** are typed as `Record<string, string>`.
- **React Query and Apollo** keep their caches in `Map`s keyed by query.
- **Log shippers** like Logstash's grok filter are, underneath, a library of named regular expressions.
