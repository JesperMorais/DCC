test("visa card", () => {
  expect(paymentLabel({ brand: "visa", cardNumber: "4111111111114242" })).toBe("Visa ending in 4242");
});

test("mastercard", () => {
  expect(paymentLabel({ brand: "mastercard", cardNumber: "5500000000000004" })).toBe("Mastercard ending in 0004");
});

test("bank transfer", () => {
  expect(paymentLabel({ iban: "SE4550000000058398257466" })).toBe("Bank transfer ending in 7466");
});

test("invoice", () => {
  expect(paymentLabel({ invoiceEmail: "billing@acme.com" })).toBe("Invoice to billing@acme.com");
});

test("never leaks the full number", () => {
  const label = paymentLabel({ brand: "visa", cardNumber: "4000123412341234" });
  expect(label.includes("4000123412341234")).toBe(false);
});
