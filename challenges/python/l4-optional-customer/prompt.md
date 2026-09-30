The shop greets logged-in customers by name on the checkout page. `Customer` is already written for you. Write two functions:

**`find_customer(customers, email)`** returns the matching `Customer`, or `None` if there's no match.

- Emails match **case-insensitively**, ignoring surrounding whitespace (on both sides of the comparison).
- Return the customer object itself, not a copy.

**`greeting_name(customers, email)`** returns the name to show:

- the customer's `nickname`, if it's set (not `None` and not empty)
- otherwise their `name`
- `"Guest"` if no customer matches

```python
customers = [Customer("ada@example.com", "Ada Lovelace", "Ada"),
             Customer("linus@example.com", "Linus Torvalds")]

find_customer(customers, " ADA@example.com")  # Customer(email='ada@example.com', ...)
find_customer(customers, "bob@example.com")   # None
greeting_name(customers, "ada@example.com")   # "Ada"
greeting_name(customers, "linus@example.com") # "Linus Torvalds"
greeting_name(customers, "bob@example.com")   # "Guest"
```
