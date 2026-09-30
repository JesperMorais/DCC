### Variables

A **variable** is a name for a value. You create one with `=`:

```python
minutes = 90
hours = minutes / 60   # 1.5
```

Read `=` as "store the value on the right under the name on the left". From then on, `hours` means `1.5`.

### Doing maths

Python works like a calculator: `+` adds, `-` subtracts, `*` multiplies and `/` divides. Brackets work the way they do in maths:

```python
width = 4.0
height = 2.5
area = width * height          # 10.0
average = (3 + 4 + 8) / 3      # 5.0
```

### `int` and `float`

Python has two kinds of numbers. An **`int`** is a whole number (`3`, `-10`, `0`). A **`float`** is a number with a decimal point (`2.5`, `0.15`, `100.0`). Dividing with `/` always gives a `float`, even for `10 / 2` (that's `5.0`).

### Functions, briefly

A **function** takes inputs (**parameters**) and gives back a result with `return`. In `def f(x: float) -> float:`, the parts after the colons are **type hints**. They say what type each input is and what comes out.
