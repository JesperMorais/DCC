### Values vs references

Numbers and strings are copied when you assign them. Arrays and objects are not: the variable holds a **reference**, and assigning it hands out a second name for the *same* thing.

```ts
const a = [1, 2, 3];
const b = a;        // not a copy
b.push(4);
a;                  // [1, 2, 3, 4]
a === b;            // true
```

To change something without touching the original, copy first, then change the copy:

```ts
const c = [...a];             // new array, same items
const user = { name: "Ada", age: 36 };
const older = { ...user, age: 37 };   // new object, user unchanged
```

A `readonly T[]` parameter stops *you* from calling `push` on it, but it doesn't stop you from passing the same array along under another name.

### Debugging technique: check identity

When "old" data changes by itself, two names are probably pointing at one object. Keep a reference to the input, run the function, then compare: `input === output`, or log the input before and after the call. If the input changed, follow the variable back to where the copy was *supposed* to happen.
