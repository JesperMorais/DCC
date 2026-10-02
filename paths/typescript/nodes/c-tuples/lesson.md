React's most famous line is a tuple:

```ts
const [count, setCount] = useState(0);
```

`useState` returns **two** things, a value and a function, and you name them whatever you like. If it returned `(number | Function)[]` instead, TypeScript couldn't tell you that `count` is a number and `setCount` is a function. This lesson is about that difference, and about writing down what your functions return.

### Arrays vs tuples

An **array type** says "any number of items, all of this type":

```ts
const temps: number[] = [12, -3, 7]; // length unknown
```

A **tuple type** says "exactly these positions, each with its own type":

```ts
const point: [number, number] = [59.33, 18.06];
const entry: [string, number] = ["Ada", 36];

entry[0].toUpperCase(); // ✓ string
entry[1].toFixed(1);    // ✓ number
entry[2];               // ✗ Tuple type '[string, number]' of length '2' has no element at index '2'
```

At runtime a tuple is just an array. The difference is all in the type: the compiler knows the length and the type at each index.

You can **label** the positions so editors show what each one means. Labels are documentation only, and `[lat: number, lng: number]` is the same type as `[number, number]`:

```ts
type Coord = [lat: number, lng: number];
```

### TypeScript won't guess "tuple" for you

This is the step people miss. An array literal is inferred as an **array**, not a tuple:

```ts
const pair = [1920, 1080];          // number[], not [number, number]
const mixed = ["Ada", 36];          // (string | number)[]
```

`mixed[0]` is now `string | number`, and you've lost the information. To get a tuple, say so: annotate the variable, give the function a return type, or use `as const` (which also makes it `readonly` with literal types):

```ts
const size: [number, number] = [1920, 1080];
const fixed = [1920, 1080] as const;  // readonly [1920, 1080]
```

### Return types: write down the promise

TypeScript infers return types, but writing them down has two benefits. Callers see the promise in the signature, and the compiler checks your body **against** the promise instead of just believing it:

```ts
function minMax(nums: readonly number[]): [number, number] | null {
  if (nums.length === 0) return null;
  return [Math.min(...nums), Math.max(...nums)]; // ✓ checked as a tuple
}
```

Without the annotation, that `return [...]` would be inferred as `number[]`, and the caller would get `number[] | null`. With it, the array literal is checked **as** a tuple.

A return type also catches the forgotten branch:

```ts
function label(n: number): string {
  if (n > 0) return "positive";
  if (n < 0) return "negative";
} // ✗ Function lacks ending return statement and return type does not include 'undefined'.
```

Two return types worth knowing: `void` means "returns nothing useful" (like a click handler), and `T | null` (or `T | undefined`) means "might not have an answer". Use `null` for "no result" instead of returning a fake value like `-1` or `[0, 0]`. The caller is then forced to check.

### Destructuring: the other half

The caller pulls a tuple apart by position:

```ts
const result = minMax(temps);
if (result) {
  const [low, high] = result;      // low: number, high: number
}
```

You can skip positions with a comma: `const [, high] = result;`. Destructuring also works in parameters: `function area([w, h]: [number, number]) { return w * h; }`.

### Tuple or object?

Tuples are great for **two or three** values that are always used together and whose order is obvious, such as a pair of coordinates, a `[key, value]` entry or `[value, setter]`. Once there are more fields, or the order isn't obvious (is it `[width, height]` or `[height, width]`?), return an object with named properties instead. `{ width, height }` can't be read the wrong way round.

### Gotchas

- **Tuples aren't frozen.** `point.push(3)` compiles on a mutable tuple, and then the type lies about the length. Use `readonly [number, number]` when you hand one out.
- **`Object.entries` gives you `[string, T][]`**, an array of tuples. That's why `for (const [key, value] of Object.entries(obj))` works.
- **Don't use `any[]` as a return type.** The caller gets nothing. Write the tuple.
- **`Number("")` is `0`, not `NaN`.** When you parse numbers out of strings, check for empty pieces yourself.

### In the wild

- **React hooks**: `useState` returns `[value, setValue]`, and `useReducer` returns `[state, dispatch]`.
- **`Object.entries` and `new Map(entries)`** both speak in `[key, value]` tuples.
- **Go-style results** in some TypeScript codebases: `const [err, data] = await to(fetchUser())`.
- **Image and layout code** passes sizes and points around as `[width, height]` and `[x, y]`, which is what you'll do in the lab.
