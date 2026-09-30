const products: readonly Product[] = [
  { sku: "A-1", name: "Mug", stock: 12 },
  { sku: "B-7", name: "Poster", stock: 2 },
  { sku: "C-3", name: "Cap", stock: 0 },
  { sku: "D-4", name: "Badge", stock: 2 },
];

test("filters, sorts and formats", () => {
  expect(lowStockReport(products, 5)).toEqual([
    "C-3 Cap (out of stock)",
    "D-4 Badge (2 left)",
    "B-7 Poster (2 left)",
  ]);
});

test("threshold is strict: stock equal to it is fine", () => {
  expect(lowStockReport(products, 2)).toEqual(["C-3 Cap (out of stock)"]);
});

test("nothing below the threshold → empty list", () => {
  expect(lowStockReport(products, 0)).toEqual([]);
});

test("a high threshold includes everything", () => {
  expect(lowStockReport(products, 100)).toHaveLength(4);
  expect(lowStockReport(products, 100)[3]).toBe("A-1 Mug (12 left)");
});

test("does not reorder the input", () => {
  lowStockReport(products, 100);
  expect(products.map((p) => p.sku)).toEqual(["A-1", "B-7", "C-3", "D-4"]);
});

test("empty catalogue", () => {
  expect(lowStockReport([], 10)).toEqual([]);
});
