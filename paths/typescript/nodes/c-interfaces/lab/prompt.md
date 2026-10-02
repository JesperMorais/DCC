You're building a small job board. Companies post listings through a form, and most of them skip the optional fields. Model the listings and write three helpers for the recruiters' dashboard.

**Step 1: the interfaces.** Replace the two placeholders in the starter:

- `Listing` has `id` (number), `title` (string) and `company` (string), plus two **optional** fields:
  - `salary`: a number (yearly)
  - `openings`: a number (how many people they still want to hire)
- `FeaturedListing` **extends** `Listing` with one more required field, `badge` (string).

**Step 2: the functions.**

**`openingsLeft(listing)`** returns the number of open positions. If `openings` is missing, the company didn't say, so assume **1**. A listing with `openings: 0` is filled and must give `0`.

```ts
openingsLeft({ id: 1, title: "Barista", company: "Bean There" });              // 1
openingsLeft({ id: 2, title: "Chef", company: "Pan Am", openings: 3 });       // 3
openingsLeft({ id: 3, title: "Waiter", company: "Pan Am", openings: 0 });     // 0
```

**`hire(listing)`** records one hire. It returns a **new** listing with `openings` set to one less than `openingsLeft(listing)`, never below `0`. Every other field is kept as it was, and the original listing must not change.

```ts
hire({ id: 1, title: "Barista", company: "Bean There" });
// { id: 1, title: "Barista", company: "Bean There", openings: 0 }
```

**`feature(listing, badge)`** returns a new `FeaturedListing`: the same fields as `listing` plus `badge`. The original must not change.

```ts
feature({ id: 2, title: "Chef", company: "Pan Am", salary: 52000 }, "Hot");
// { id: 2, title: "Chef", company: "Pan Am", salary: 52000, badge: "Hot" }
```
