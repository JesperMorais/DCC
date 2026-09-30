Write **`Getters<T>`**: for every **string** key of `T`, produce a method named `get` + the capitalised key that returns that property's type.

```ts
interface Person {
  name: string;
  age: number;
  isAdmin: boolean;
}

type PersonGetters = Getters<Person>;
// {
//   getName: () => string;
//   getAge: () => number;
//   getIsAdmin: () => boolean;
// }
```

- Keys that are **not strings** (number keys like `0`, symbol keys) are dropped.
- `Getters<{}>` is `{}`.
