interface Ticket {
  id: number;
  title: string;
  createdAt: number;
}

interface User {
  name: string;
  age: number;
  admin: boolean;
}

const tickets: Ticket[] = [
  { id: 1, title: "Login broken", createdAt: 1_700_000_100 },
  { id: 2, title: "Typo on pricing page", createdAt: 1_700_000_900 },
  { id: 3, title: "Export is slow", createdAt: 1_700_000_500 },
];

const users: User[] = [
  { name: "Ada", age: 36, admin: true },
  { name: "Linus", age: 28, admin: false },
  { name: "Grace", age: 45, admin: true },
];

const latest = newest(tickets);
const grace = findBy(users, "name", "Grace");

type cases = [
  Expect<Equal<typeof latest, Ticket | undefined>>,
  Expect<Equal<typeof grace, User | undefined>>,
];

// @ts-expect-error — items need a numeric createdAt
newest([{ id: 1 }]);

// @ts-expect-error — "emial" is not a key of User
findBy(users, "emial", "ada@example.com");

// @ts-expect-error — age is a number, not a string
findBy(users, "age", "36");

test("newest picks the largest createdAt", () => {
  expect(latest).toEqual({ id: 2, title: "Typo on pricing page", createdAt: 1_700_000_900 });
});

test("newest of an empty list is undefined", () => {
  expect(newest([])).toBe(undefined);
});

test("newest keeps the first of a tie", () => {
  expect(newest([{ tag: "a", createdAt: 5 }, { tag: "b", createdAt: 5 }])).toEqual({ tag: "a", createdAt: 5 });
});

test("findBy finds by string field", () => {
  expect(grace).toEqual({ name: "Grace", age: 45, admin: true });
});

test("findBy finds by number and boolean fields", () => {
  expect(findBy(users, "age", 28)).toEqual({ name: "Linus", age: 28, admin: false });
  expect(findBy(users, "admin", true)).toEqual({ name: "Ada", age: 36, admin: true });
});

test("findBy returns undefined when nothing matches", () => {
  expect(findBy(users, "age", 99)).toBe(undefined);
});

test("neither function modifies the input", () => {
  newest(tickets);
  findBy(users, "name", "Ada");
  expect(tickets.map((t) => t.id)).toEqual([1, 2, 3]);
  expect(users).toHaveLength(3);
});
