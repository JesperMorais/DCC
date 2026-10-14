**Bug report from a reader:** "I'm doing the reading challenge: 1 page on day 1, 2 pages on day 2, 3 pages on day 3, and so on. After 3 days the app says I've read 3 pages. I've read 6!"

`total_pages_read(days)` returns the total number of pages read after `days` days, where on day *n* you read *n* pages. Zero days means zero pages.

- `total_pages_read(3)` → `6` (1 + 2 + 3)
- `total_pages_read(1)` → `1`
- `total_pages_read(0)` → `0`

**Reproduction:** `total_pages_read(3)` returns `3`, expected `6`.
