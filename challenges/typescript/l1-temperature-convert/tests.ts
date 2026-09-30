test("freezing point", () => {
  expect(celsiusToFahrenheit(0)).toBe(32);
});

test("boiling point", () => {
  expect(celsiusToFahrenheit(100)).toBe(212);
});

test("minus forty is the same on both scales", () => {
  expect(celsiusToFahrenheit(-40)).toBe(-40);
});

test("a nice room temperature", () => {
  expect(celsiusToFahrenheit(20)).toBe(68);
});

test("a warm summer day", () => {
  expect(celsiusToFahrenheit(25)).toBe(77);
});
