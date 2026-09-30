On the "Your orders" page, each order shows how it was paid. A customer's payment method is one of three shapes (see the starter):

- `Card` — has `cardNumber` and `brand`
- `BankTransfer` — has `iban`
- `Invoice` — has `invoiceEmail`

Write `paymentLabel(method)` that returns a short, safe label. Never show the full card number or IBAN — only the **last 4 characters**.

```ts
paymentLabel({ brand: "visa", cardNumber: "4111111111114242" });
// "Visa ending in 4242"

paymentLabel({ brand: "mastercard", cardNumber: "5500000000000004" });
// "Mastercard ending in 0004"

paymentLabel({ iban: "SE4550000000058398257466" });
// "Bank transfer ending in 7466"

paymentLabel({ invoiceEmail: "billing@acme.com" });
// "Invoice to billing@acme.com"
```
