function makeCatalog(): Product[] {
  return [
    { name: "Notebook", priceCents: 1200 },
    { name: "Pen", priceCents: 250 },
    { name: "Backpack", priceCents: 4500 },
  ];
}

test("orders products from cheapest to most expensive", () => {
  expect(cheapestFirst(makeCatalog()).map((p) => p.name)).toEqual(["Pen", "Notebook", "Backpack"]);
});

test("an empty catalog", () => {
  expect(cheapestFirst([])).toEqual([]);
});

test("equal prices keep their original order", () => {
  const items: Product[] = [
    { name: "Red mug", priceCents: 800 },
    { name: "Sticker", priceCents: 100 },
    { name: "Blue mug", priceCents: 800 },
  ];
  expect(cheapestFirst(items).map((p) => p.name)).toEqual(["Sticker", "Red mug", "Blue mug"]);
});

test("leaves the catalog in its original order", () => {
  const catalog = makeCatalog();
  cheapestFirst(catalog);
  expect(catalog.map((p) => p.name)).toEqual(["Notebook", "Pen", "Backpack"]);
});

test("sorting twice gives the same answer and still leaves the catalog alone", () => {
  const catalog = makeCatalog();
  const first = cheapestFirst(catalog);
  const second = cheapestFirst(catalog);
  expect(second.map((p) => p.name)).toEqual(first.map((p) => p.name));
  expect(catalog[0].name).toBe("Notebook");
});
