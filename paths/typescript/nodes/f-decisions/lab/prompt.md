The shop's shipping rules live in a wiki page nobody reads. Turn them into code.

**`freeShipping(subtotal, weightKg, isMember)`** returns a `boolean`. An order ships free when:

- the customer is a member, **or** the subtotal is **$50 or more**,
- **and** the parcel weighs **20 kg or less**. Heavier parcels go by freight and are never free.

**`shippingCost(subtotal, weightKg, isMember)`** returns the price in dollars:

| Situation | Cost |
|---|---|
| ships free (see above) | `0` |
| over 20 kg (freight) | `25` |
| up to and including 1 kg | `4` |
| up to and including 5 kg | `8` |
| anything else | `12` |

```ts
freeShipping(60, 3, false);  // true   (big order)
freeShipping(20, 3, true);   // true   (member)
freeShipping(60, 25, true);  // false  (freight is never free)

shippingCost(20, 0.5, false); // 4
shippingCost(20, 5, false);   // 8
shippingCost(20, 5.5, false); // 12
shippingCost(80, 30, false);  // 25
```

`subtotal` and `weightKg` are `number`s, and `isMember` is a `boolean`.
