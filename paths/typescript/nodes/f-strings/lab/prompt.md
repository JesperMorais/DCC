The checkout team needs three small text helpers.

**`maskCard(card)`** hides a card number on the receipt. Keep the **last four** characters and replace every character before them with `*`, so the result has the same length as the input. A number of four characters or fewer is returned unchanged.

```ts
maskCard("4242424242424242"); // "************4242"
maskCard("123456");           // "**3456"
maskCard("42");               // "42"
```

**`slugify(title)`** turns a product title into a URL slug: remove the spaces around it, make it lowercase, and replace each space with `-`. (Words are separated by single spaces.)

```ts
slugify("  Blue Coffee Mug ");  // "blue-coffee-mug"
slugify("Gift Card");           // "gift-card"
```

**`isPdf(filename)`** is `true` if the file name ends with `.pdf`, in any case.

```ts
isPdf("invoice.pdf");  // true
isPdf("SCAN.PDF");     // true
isPdf("pdf-notes.txt"); // false
```

All three take a `string`. `maskCard` and `slugify` return a `string`, and `isPdf` returns a `boolean`.
