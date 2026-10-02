type cases = [
  Expect<Equal<ReturnType<typeof maskCard>, string>>,
  Expect<Equal<ReturnType<typeof slugify>, string>>,
  Expect<Equal<ReturnType<typeof isPdf>, boolean>>,
];

// Only type-checked, never run:
void (() => {
  // @ts-expect-error a card number is text, not a number
  maskCard(4242424242424242);
});

test("maskCard keeps only the last four", () => {
  expect(maskCard("4242424242424242")).toBe("************4242");
  expect(maskCard("123456")).toBe("**3456");
});

test("maskCard keeps the length", () => {
  expect(maskCard("378282246310005")).toHaveLength(15);
});

test("maskCard leaves short numbers alone", () => {
  expect(maskCard("1234")).toBe("1234");
  expect(maskCard("42")).toBe("42");
});

test("slugify trims, lowercases and joins with dashes", () => {
  expect(slugify("  Blue Coffee Mug ")).toBe("blue-coffee-mug");
  expect(slugify("Gift Card")).toBe("gift-card");
});

test("slugify of a single word", () => {
  expect(slugify("MUGS")).toBe("mugs");
});

test("isPdf ignores case", () => {
  expect(isPdf("invoice.pdf")).toBe(true);
  expect(isPdf("SCAN.PDF")).toBe(true);
});

test("isPdf only looks at the end", () => {
  expect(isPdf("pdf-notes.txt")).toBe(false);
  expect(isPdf("report.pdf.zip")).toBe(false);
});
