A coffee shop's ordering screen prints a short summary of each order. Most people want a medium drink with one espresso shot, so those are the **defaults**.

Write `order_summary(drink, size="medium", shots=1)` that returns `"<size> <drink>, <shots> shot"`:

- `size` defaults to `"medium"` and `shots` defaults to `1` when the caller leaves them out.
- Write `"shot"` for exactly 1 shot and `"shots"` for any other number, including 0.

Examples:

- `order_summary("latte")` → `"medium latte, 1 shot"`
- `order_summary("mocha", "large", 2)` → `"large mocha, 2 shots"`
- `order_summary("americano", shots=0)` → `"medium americano, 0 shots"`

The starter's `def` line has no defaults yet. Adding them is part of the task.
