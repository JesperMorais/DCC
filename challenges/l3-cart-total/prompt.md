You're building the checkout page of a web shop. Prices are stored as **whole cents** to avoid floating-point surprises.

1. Fill in the `CartItem` interface. A cart line has:
   - `name` — a string
   - `priceCents` — the price of **one** unit, in cents (number)
   - `quantity` — how many units (number)
2. Write `cartTotal(items)` that returns the total price of the cart in cents.
3. Write `formatCents(cents)` that turns cents into a dollar string with exactly two decimals.

```ts
const cart: CartItem[] = [
  { name: "Coffee", priceCents: 450, quantity: 2 },
  { name: "Bagel", priceCents: 325, quantity: 1 },
];

cartTotal(cart);      // 1225
cartTotal([]);        // 0
formatCents(1225);    // "$12.25"
formatCents(5);       // "$0.05"
```
