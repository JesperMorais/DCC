test("a child pays 5", () => {
  expect(ticketPrice(8)).toBe(5);
});

test("an adult pays 10", () => {
  expect(ticketPrice(30)).toBe(10);
});

test("someone over 65 pays 7", () => {
  expect(ticketPrice(70)).toBe(7);
});

test("exactly 65 already gets the senior price", () => {
  expect(ticketPrice(65)).toBe(7);
});

test("64 still pays full price", () => {
  expect(ticketPrice(64)).toBe(10);
});

test("11 is a child, 12 pays full price", () => {
  expect(ticketPrice(11)).toBe(5);
  expect(ticketPrice(12)).toBe(10);
});
