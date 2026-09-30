test("first five", () => {
  expect(fizzBuzz(5)).toEqual(["1", "2", "Fizz", "4", "Buzz"]);
});

test("up to 15", () => {
  expect(fizzBuzz(15)).toEqual([
    "1", "2", "Fizz", "4", "Buzz", "Fizz", "7", "8", "Fizz", "Buzz",
    "11", "Fizz", "13", "14", "FizzBuzz",
  ]);
});

test("just one", () => {
  expect(fizzBuzz(1)).toEqual(["1"]);
});

test("zero gives an empty array", () => {
  expect(fizzBuzz(0)).toEqual([]);
});

test("30 is FizzBuzz too", () => {
  const out = fizzBuzz(30);
  expect(out.length).toBe(30);
  expect(out[29]).toBe("FizzBuzz");
});
