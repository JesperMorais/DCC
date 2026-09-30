### f-strings, a quick recap

An **f-string** has an `f` before the opening quote and lets you drop values into text with `{...}`:

```python
planet = "Mars"
moons = 2
f"{planet} has {moons} moons"   # "Mars has 2 moons"
```

### Formatting numbers

After the value, you can add a colon and a **format spec** that controls how it's shown. `.Nf` means "show N digits after the decimal point", and it rounds for you:

```python
distance_km = 42.19512
f"{distance_km:.1f} km"   # "42.2 km"
f"{distance_km:.3f} km"   # "42.195 km"
f"{7:.2f}"                # "7.00"  (works on whole numbers too)
```

### Why bother? Floats aren't exact

Computers store decimals in binary, and some numbers can't be stored exactly. That's why

```python
0.1 + 0.2    # 0.30000000000000004
```

The value is *almost* right. When you show it to a human, format it to the number of decimals you actually want.
