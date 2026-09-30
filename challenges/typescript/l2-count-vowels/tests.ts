test("hello has 2 vowels", () => {
  expect(countVowels("hello")).toBe(2);
});

test("y is not a vowel", () => {
  expect(countVowels("TypeScript")).toBe(2);
});

test("uppercase vowels count too", () => {
  expect(countVowels("AEIOU")).toBe(5);
});

test("empty string has none", () => {
  expect(countVowels("")).toBe(0);
});

test("no vowels at all", () => {
  expect(countVowels("rhythm")).toBe(0);
});

test("a whole sentence", () => {
  expect(countVowels("I love coding!")).toBe(5);
});
