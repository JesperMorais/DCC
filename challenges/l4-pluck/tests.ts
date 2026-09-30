interface User {
  name: string;
  age: number;
  admin: boolean;
}

const users: User[] = [
  { name: "Ada", age: 36, admin: true },
  { name: "Linus", age: 28, admin: false },
  { name: "Grace", age: 45, admin: true },
];

const names = pluck(users, "name");
const ages = pluck(users, "age");

type cases = [
  Expect<Equal<typeof names, string[]>>,
  Expect<Equal<typeof ages, number[]>>,
];

// @ts-expect-error — "emial" is not a key of User
pluck(users, "emial");

test("plucks names", () => {
  expect(names).toEqual(["Ada", "Linus", "Grace"]);
});

test("plucks numbers", () => {
  expect(ages).toEqual([36, 28, 45]);
});

test("plucks booleans", () => {
  expect(pluck(users, "admin")).toEqual([true, false, true]);
});

test("empty list gives empty column", () => {
  const none: User[] = [];
  expect(pluck(none, "name")).toEqual([]);
});

test("works on any object type", () => {
  const points = [{ x: 1, y: 2 }, { x: 3, y: 4 }];
  expect(pluck(points, "y")).toEqual([2, 4]);
});
