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
  return "";
}
