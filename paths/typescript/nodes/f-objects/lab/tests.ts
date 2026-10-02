type coffeeCases = [
  Expect<Equal<Size, "small" | "medium" | "large">>,
  Expect<Equal<CoffeeOrder, { drink: string; size: Size; shots: number; oatMilk: boolean }>>,
];

// @ts-expect-error — "huge" is not a Size
newOrder("latte", "huge");

// @ts-expect-error — an order must say how many shots it has
const shotless: CoffeeOrder = { drink: "mocha", size: "small", oatMilk: false };

test("newOrder starts with one shot and regular milk", () => {
  expect(newOrder("latte", "medium")).toEqual({ drink: "latte", size: "medium", shots: 1, oatMilk: false });
});

test("base price depends on the size", () => {
  expect(orderPrice({ drink: "latte", size: "small", shots: 1, oatMilk: false })).toBe(30);
  expect(orderPrice({ drink: "latte", size: "medium", shots: 1, oatMilk: false })).toBe(35);
  expect(orderPrice({ drink: "latte", size: "large", shots: 1, oatMilk: false })).toBe(40);
});

test("extra shots cost 6 each, the first is included", () => {
  expect(orderPrice({ drink: "americano", size: "small", shots: 2, oatMilk: false })).toBe(36);
  expect(orderPrice({ drink: "americano", size: "small", shots: 4, oatMilk: false })).toBe(48);
});

test("oat milk costs 5", () => {
  expect(orderPrice({ drink: "cortado", size: "medium", shots: 1, oatMilk: true })).toBe(40);
});

test("everything together", () => {
  expect(orderPrice({ drink: "flat white", size: "large", shots: 3, oatMilk: true })).toBe(57);
});

test("withOatMilk switches the milk and keeps the rest", () => {
  expect(withOatMilk({ drink: "chai", size: "large", shots: 2, oatMilk: false })).toEqual({
    drink: "chai",
    size: "large",
    shots: 2,
    oatMilk: true,
  });
});

test("withOatMilk does not change the original order", () => {
  const onScreen: CoffeeOrder = { drink: "latte", size: "small", shots: 1, oatMilk: false };
  const changed = withOatMilk(onScreen);
  expect(onScreen.oatMilk).toBe(false);
  expect(changed).not.toBe(onScreen);
});
