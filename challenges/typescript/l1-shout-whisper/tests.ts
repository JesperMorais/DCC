test("shout makes it loud", () => {
  expect(shout("hello")).toBe("HELLO!");
});

test("shout works with several words", () => {
  expect(shout("Good Morning")).toBe("GOOD MORNING!");
});

test("whisper makes it quiet", () => {
  expect(whisper("HELLO")).toBe("hello...");
});

test("whisper works with mixed case", () => {
  expect(whisper("Be Quiet")).toBe("be quiet...");
});

test("shouting an empty string is just the !", () => {
  expect(shout("")).toBe("!");
});
