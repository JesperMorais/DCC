You're building a weather widget for American visitors. Write `celsius_to_fahrenheit`, which converts a temperature using

**F = C × 9 / 5 + 32**

```c
double celsius_to_fahrenheit(double celsius);
```

Examples:

- `celsius_to_fahrenheit(100.0)` → `212.0` (water boils)
- `celsius_to_fahrenheit(0.0)` → `32.0` (water freezes)
- `celsius_to_fahrenheit(37.0)` → `98.6` (body temperature)
- `celsius_to_fahrenheit(-40.0)` → `-40.0` (the one temperature where both scales agree)

Decimals matter: `21.5` °C is `70.7` °F.
