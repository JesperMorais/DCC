### `Record<K, V>` — an object used as a dictionary

When you don't know the keys in advance, but you know what type every value has, use `Record`:

```ts
const stockByColour: Record<string, number> = {};
stockByColour["red"] = 3;
stockByColour.blue = 5;
```

`Record<string, number>` means "any string key, every value is a number".

### Reading a key that might not be there

With plain `strict` settings, `stockByColour["green"]` is typed as `number` — but at runtime it's `undefined` if nobody set it. Guard against that when you *update* a value:

```ts
const current = stockByColour["green"] ?? 0;
```

### Finding all matches with a regex

```ts
"a1-b22".match(/[0-9]+/g); // ["1", "22"]
"abc".match(/[0-9]+/g);    // null  ← no matches
```

The `g` flag returns every match; the result is `string[] | null`, so plan for `null`.
