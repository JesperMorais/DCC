interface Card {
  brand: "visa" | "mastercard";
  cardNumber: string;
}

interface BankTransfer {
  iban: string;
}

interface Invoice {
  invoiceEmail: string;
}

type PaymentMethod = Card | BankTransfer | Invoice;

function paymentLabel(method: PaymentMethod): string {
  if ("cardNumber" in method) {
    const brand = method.brand === "visa" ? "Visa" : "Mastercard";
    return `${brand} ending in ${method.cardNumber.slice(-4)}`;
  }
  if ("iban" in method) {
    return `Bank transfer ending in ${method.iban.slice(-4)}`;
  }
  return `Invoice to ${method.invoiceEmail}`;
}
