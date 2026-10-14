test("a guest in the middle of the list", () => {
  expect(isOnGuestList(["Ana", "Leo", "Kim"], "Leo")).toBe(true);
});

test("the last guest", () => {
  expect(isOnGuestList(["Ana", "Leo", "Kim"], "Kim")).toBe(true);
});

test("the first guest", () => {
  expect(isOnGuestList(["Ana", "Leo", "Kim"], "Ana")).toBe(true);
});

test("someone who wasn't invited", () => {
  expect(isOnGuestList(["Ana", "Leo", "Kim"], "Brad")).toBe(false);
});

test("an empty guest list", () => {
  expect(isOnGuestList([], "Ana")).toBe(false);
});
