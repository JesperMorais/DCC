test("the bus delivers payloads to every listener in order", () => {
  const bus = new EventBus<{ ping: number }>();
  const got: string[] = [];
  bus.on("ping", (n) => got.push(`first ${n}`));
  bus.on("ping", (n) => got.push(`second ${n}`));
  bus.emit("ping", 7);
  expect(got).toEqual(["first 7", "second 7"]);
});

test("onAdded called directly adds the quantity", () => {
  const badge = new CartBadge(new EventBus<ShopEvents>());
  badge.onAdded({ sku: "mug", qty: 2 });
  expect(badge.count).toBe(2);
});

test("emitting added updates the count", () => {
  const bus = new EventBus<ShopEvents>();
  const badge = new CartBadge(bus);
  bus.emit("added", { sku: "mug", qty: 2 });
  bus.emit("added", { sku: "tea", qty: 1 });
  expect(badge.count).toBe(3);
});

test("emitting cleared resets the count", () => {
  const bus = new EventBus<ShopEvents>();
  const badge = new CartBadge(bus);
  badge.count = 5;
  bus.emit("cleared", undefined);
  expect(badge.count).toBe(0);
});

test("two badges on one bus both stay in sync", () => {
  const bus = new EventBus<ShopEvents>();
  const header = new CartBadge(bus);
  const sidebar = new CartBadge(bus);
  bus.emit("added", { sku: "mug", qty: 4 });
  expect(header.count).toBe(4);
  expect(sidebar.count).toBe(4);
});

test("add, clear, add again", () => {
  const bus = new EventBus<ShopEvents>();
  const badge = new CartBadge(bus);
  bus.emit("added", { sku: "mug", qty: 2 });
  bus.emit("cleared", undefined);
  bus.emit("added", { sku: "tea", qty: 1 });
  expect(badge.count).toBe(1);
});
