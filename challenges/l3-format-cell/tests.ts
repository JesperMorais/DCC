test("null becomes N/A", () => {
  expect(formatCell(null)).toBe("N/A");
});

test("numbers get two decimals", () => {
  expect(formatCell(3)).toBe("3.00");
  expect(formatCell(2.5)).toBe("2.50");
  expect(formatCell(0)).toBe("0.00");
});

test("booleans become Yes/No", () => {
  expect(formatCell(true)).toBe("Yes");
  expect(formatCell(false)).toBe("No");
});

test("strings are trimmed", () => {
  expect(formatCell("  Ada ")).toBe("Ada");
});

test("blank strings become N/A", () => {
  expect(formatCell("   ")).toBe("N/A");
  expect(formatCell("")).toBe("N/A");
});

test("formats a whole row", () => {
  expect(formatRow(["Ada", 36, false, null])).toBe("Ada | 36.00 | No | N/A");
});

test("an empty row is an empty string", () => {
  expect(formatRow([])).toBe("");
});
