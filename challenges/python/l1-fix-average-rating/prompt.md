**Bug report from the shop team:** "Our product pages show suspiciously round star ratings. A lamp with two reviews, a 4 and a 5, shows **4 stars**. It should show 4.5!"

`average_rating(total_stars, review_count)` takes the **sum of all stars** a product got and **how many reviews** there were, and returns the average as a decimal number. A product with no reviews has an average of `0.0`.

- `average_rating(10, 2)` → `5.0`
- `average_rating(9, 2)` → `4.5`
- `average_rating(0, 0)` → `0.0`

**Reproduction:** `average_rating(9, 2)` returns `4`, expected `4.5`.
