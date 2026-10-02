Your shop's order page shows a one-line tracking update for each parcel. The carrier API sends a status that is one of five shapes, tagged by `kind` (see `ShipmentStatus` in the starter).

**1. `assertNever(value)`** takes a value of type `never`, and always throws an `Error` whose message is `Unexpected value: ` followed by `JSON.stringify(value)`. Its return type is `never`.

**2. `trackingLine(status)`** returns:

| `kind` | Line |
|---|---|
| `"label_created"` | `Label created, waiting for pickup` |
| `"in_transit"` | `In transit with <carrier>, last seen in <location>` |
| `"out_for_delivery"` | `Out for delivery, expected by <eta>` |
| `"delivered"` | `Delivered, signed by <signedBy>`, or just `Delivered` when `signedBy` is `null` |
| `"returned"` | `Returned to sender: <reason>` |

**3. `isFinal(status)`** returns `true` for `"delivered"` and `"returned"` (nothing more will happen), `false` for the other three.

Both functions must be **exhaustive**: use `assertNever` in the `default` of a `switch`, so adding a sixth kind later is a compile error. At runtime, a status with an unknown `kind` (bad data from the API) must make both functions throw.

```ts
trackingLine({ kind: "in_transit", carrier: "PostNord", location: "Malmö" });
// "In transit with PostNord, last seen in Malmö"
trackingLine({ kind: "delivered", signedBy: null }); // "Delivered"
isFinal({ kind: "returned", reason: "Address not found" }); // true
```
