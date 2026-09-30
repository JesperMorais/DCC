type personCases = [Expect<Equal<Person, { name: string; age: number }>>];

// @ts-expect-error — a Person must have an age
const ageless: Person = { name: "Nobody" };

test("describe Ada", () => {
  expect(describe({ name: "Ada", age: 36 })).toBe("Ada is 36 years old");
});

test("describe a baby", () => {
  expect(describe({ name: "Mo", age: 0 })).toBe("Mo is 0 years old");
});

test("haveBirthday adds one year", () => {
  expect(haveBirthday({ name: "Linus", age: 20 })).toEqual({ name: "Linus", age: 21 });
});

test("haveBirthday keeps the name", () => {
  expect(haveBirthday({ name: "Grace", age: 85 }).name).toBe("Grace");
});

test("haveBirthday does not change the original", () => {
  const p: Person = { name: "Alan", age: 41 };
  const older = haveBirthday(p);
  expect(p.age).toBe(41);
  expect(older.age).toBe(42);
});
