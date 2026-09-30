The roller coaster at the amusement park has a sign at the entrance:

> You must be **at least 120 cm tall** and **at least 8 years old** to ride.

Write a function `can_ride` that returns `True` if the visitor is allowed on and `False` if not.

- `can_ride(130, 10)` → `True`
- `can_ride(120, 8)` → `True` (exactly on the limits is fine)
- `can_ride(150, 7)` → `False` (tall enough, but too young)

The signature is `def can_ride(height_cm: int, age: int) -> bool:`. `bool` is the type for `True`/`False`.
