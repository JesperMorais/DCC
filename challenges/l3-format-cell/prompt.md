You're exporting a spreadsheet-like report to plain text. Each cell can hold a `string`, a `number`, a `boolean` or `null`.

Write `formatCell(value)` with these rules:

- `null` → `"N/A"`
- a number → always two decimals: `3` → `"3.00"`, `2.5` → `"2.50"`
- a boolean → `"Yes"` or `"No"`
- a string → trimmed of surrounding whitespace; if nothing is left, `"N/A"`

Then write `formatRow(row)` that formats every cell and joins them with `" | "`.

```ts
formatCell(null);        // "N/A"
formatCell(3);           // "3.00"
formatCell(true);        // "Yes"
formatCell("  Ada ");    // "Ada"
formatRow(["Ada", 36, false, null]); // "Ada | 36.00 | No | N/A"
```
