### Parsing text line by line

Config files are plain text, so a parser usually splits them into lines, skips the ones that don't matter, and breaks each remaining line into parts:

```ts
const text = "red: #f00\ngreen: #0f0";
for (const line of text.split("\n")) {
  const [name, hex] = line.split(": ");
  // name = "red", hex = "#f00"
}
```

`Record<string, string>` is the type of an object used as a dictionary: any string key, string value.

### `split` and destructuring

`split` cuts at **every** occurrence of the separator, and array destructuring picks pieces by position. Any pieces you don't name are silently dropped:

```ts
const [a, b] = "x,y,z".split(","); // a = "x", b = "y", "z" is gone
const [first, ...others] = "x,y,z".split(","); // others = ["y", "z"]
```

`join` is the reverse of `split`: `["y", "z"].join(",")` gives `"y,z"`.

### Debugging: hunt for the special character

When a parser fails on some inputs but not others, compare a working input with a failing one and ask what's **unusual** in the failing one: an extra separator, a space, an empty part. Then build the smallest input that still fails, like `"A=b=c"`, and trace it.
