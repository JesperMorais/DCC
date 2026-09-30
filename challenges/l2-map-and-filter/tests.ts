test("shoutAll uppercases every word", () => {
  expect(shoutAll(["hi", "there"])).toEqual(["HI", "THERE"]);
});

test("shoutAll on an empty array", () => {
  expect(shoutAll([])).toEqual([]);
});

test("longWords keeps words of at least minLength", () => {
  expect(longWords(["a", "tree", "is", "green"], 4)).toEqual(["tree", "green"]);
});

test("longWords can return nothing", () => {
  expect(longWords(["cat", "dog"], 5)).toEqual([]);
});

test("longWords with minLength 0 keeps everything", () => {
  expect(longWords(["", "x"], 0)).toEqual(["", "x"]);
});

test("the original array is not changed", () => {
  const words = ["one", "three"];
  shoutAll(words);
  longWords(words, 4);
  expect(words).toEqual(["one", "three"]);
});
