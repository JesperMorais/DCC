test("greets Ada", () => {
  expect(greet("Ada")).toBe("Hello, Ada!");
});

test("greets Linus", () => {
  expect(greet("Linus")).toBe("Hello, Linus!");
});

test("handles an empty name", () => {
  expect(greet("")).toBe("Hello, stranger!");
});
