test("counts repeated words", () => {
  expect(wordFrequency("App is slow. Very slow!")).toEqual({ app: 1, is: 1, slow: 2, very: 1 });
});

test("is case-insensitive", () => {
  expect(wordFrequency("Great GREAT great")).toEqual({ great: 3 });
});

test("digits count as words", () => {
  expect(wordFrequency("Error 404, error 500")).toEqual({ error: 2, "404": 1, "500": 1 });
});

test("newlines and punctuation separate words", () => {
  expect(wordFrequency("love it\nlove-it;LOVE")).toEqual({ love: 3, it: 2 });
});

test("no words → empty object", () => {
  expect(wordFrequency("  ...  ")).toEqual({});
  expect(wordFrequency("")).toEqual({});
});
