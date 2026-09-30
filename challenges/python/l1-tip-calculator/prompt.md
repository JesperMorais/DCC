You're splitting the bill at a restaurant and want to know what to pay **including the tip**.

Write a function `bill_with_tip` that takes the **bill** and the **tip percentage**, and returns the total (bill + tip).

- `bill_with_tip(100.0, 15.0)` → `115.0`
- `bill_with_tip(40.0, 20.0)` → `48.0`
- `bill_with_tip(25.0, 0.0)` → `25.0` (no tip)

The first line, `def bill_with_tip(bill: float, tip_percent: float) -> float:`, is already written for you. Both inputs are decimal numbers (`float`), and so is the answer. You don't need to round.
