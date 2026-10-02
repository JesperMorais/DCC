type Size = "small" | "medium" | "large";

type CoffeeOrder = {
  drink: string;
  size: Size;
  shots: number;
  oatMilk: boolean;
};

function newOrder(drink: string, size: Size): CoffeeOrder {
  return { drink, size, shots: 1, oatMilk: false };
}

function orderPrice(order: CoffeeOrder): number {
  let price = 30;
  if (order.size === "medium") price = 35;
  else if (order.size === "large") price = 40;

  price += (order.shots - 1) * 6;
  if (order.oatMilk) price += 5;
  return price;
}

function withOatMilk(order: CoffeeOrder): CoffeeOrder {
  return { drink: order.drink, size: order.size, shots: order.shots, oatMilk: true };
}
