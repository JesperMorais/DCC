test("a single number", () => {
  expect(evaluate("42")).toBe(42);
  expect(evaluate("  7 ")).toBe(7);
});

test("multi-digit numbers and whitespace", () => {
  expect(evaluate("12 + 30")).toBe(42);
  expect(evaluate("100-1")).toBe(99);
});

test("* and / before + and -", () => {
  expect(evaluate("3 + 4 * 2")).toBe(11);
  expect(evaluate("2*3+4*5")).toBe(26);
  expect(evaluate("20 - 12 / 4")).toBe(17);
});

test("equal precedence goes left to right", () => {
  expect(evaluate("10 - 2 - 3")).toBe(5);
  expect(evaluate("8 / 4 / 2")).toBe(1);
  expect(evaluate("2 * 9 / 3 * 2")).toBe(12);
});

test("division can produce fractions", () => {
  expect(evaluate("1 + 7 / 2")).toBe(4.5);
});

test("throws on unknown characters", () => {
  expect(() => evaluate("3 + x")).toThrow();
  expect(() => evaluate("2 ^ 3")).toThrow();
});

test("throws on malformed expressions", () => {
  expect(() => evaluate("")).toThrow();
  expect(() => evaluate("3 +")).toThrow();
  expect(() => evaluate("* 2")).toThrow();
  expect(() => evaluate("3 4")).toThrow();
  expect(() => evaluate("1 + + 2")).toThrow();
});
