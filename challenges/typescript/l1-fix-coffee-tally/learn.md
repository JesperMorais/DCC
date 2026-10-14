### Strings and numbers are different types

A **string** is text, written in quotes. A **number** is a value you can do maths with. They can look the same on screen but behave differently:

```ts
const a: string = "7";
const b: number = 7;
```

### `+` does two different jobs

With two numbers, `+` **adds**. With strings, `+` **glues them together** (this is called *concatenation*):

```ts
4 + 4;       // 8
"4" + "4";   // "44"
"Hi " + "Bo" // "Hi Bo"
```

`Number(...)` turns a string into a number: `Number("12")` is `12`. Where you call it matters: converting **after** gluing gives a different answer than converting **before** adding.

### Debugging: look at the wrong answer

A wrong result is a clue. If you expected `8` and got `44`, ask yourself: *how could anyone get 44 out of 4 and 4?* Answering that question usually points straight at the line that misbehaves.
