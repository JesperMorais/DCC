test("Ada Lovelace", () => {
  expect(initials("Ada", "Lovelace")).toBe("A.L.");
});

test("lowercase names give capital initials", () => {
  expect(initials("grace", "hopper")).toBe("G.H.");
});

test("mixed case", () => {
  expect(initials("Linus", "torvalds")).toBe("L.T.");
});

test("one-letter names", () => {
  expect(initials("x", "y")).toBe("X.Y.");
});
