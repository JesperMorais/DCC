### Narrowing object unions with `in`

When every member of a union is an object, `typeof` says `"object"` for all of them. Instead, ask *which property is present* with the `in` operator:

```ts
interface Dog { bark(): string }
interface Fish { swimSpeed: number }
type Pet = Dog | Fish;

function describe(pet: Pet): string {
  if ("bark" in pet) {
    return pet.bark();           // pet: Dog
  }
  return `swims at ${pet.swimSpeed} km/h`; // pet: Fish
}
```

After `"bark" in pet` is true, TypeScript knows `pet` is a `Dog`. In the `else`/after-return part, `Dog` has been ruled out, so only `Fish` is left. With three members, each check removes one more option.

This works best when the property names are **unique** to one member of the union.
