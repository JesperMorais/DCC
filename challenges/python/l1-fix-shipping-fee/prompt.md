**Bug report from customer support:** "A customer ordered for 60 euros and was charged shipping. Our website says shipping is free from 50 euros!"

`shipping_fee(order_total)` returns the shipping cost for an order:

- **50.0 or more:** free shipping, `0.0`
- **20.0 or more** (but under 50): `4.99`
- **under 20.0:** `7.99`

Examples:

- `shipping_fee(12.0)` → `7.99`
- `shipping_fee(35.0)` → `4.99`
- `shipping_fee(80.0)` → `0.0`

**Reproduction:** `shipping_fee(60.0)` returns `4.99`, expected `0.0`.
