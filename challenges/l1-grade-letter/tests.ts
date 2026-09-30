test("95 is an A", () => {
  expect(letterGrade(95)).toBe("A");
});

test("a perfect 100 is an A", () => {
  expect(letterGrade(100)).toBe("A");
});

test("exactly 80 is a B", () => {
  expect(letterGrade(80)).toBe("B");
});

test("72 is a C", () => {
  expect(letterGrade(72)).toBe("C");
});

test("60 is a D, 59 is an F", () => {
  expect(letterGrade(60)).toBe("D");
  expect(letterGrade(59)).toBe("F");
});

test("12 is an F", () => {
  expect(letterGrade(12)).toBe("F");
});
