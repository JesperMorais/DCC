### Making decisions with `if`

An `if` statement runs a block of code only when a condition is `True`. The block is the indented lines underneath:

```python
if speed > 50:
    print("Slow down!")
```

### More than two choices: `elif` and `else`

`elif` ("else if") adds another check, and `else` catches everything left over. Python tries them **from top to bottom** and runs only the **first** one that matches:

```python
def describe_weather(temp_c: int) -> str:
    if temp_c < 0:
        return "freezing"
    elif temp_c < 15:
        return "chilly"
    elif temp_c < 25:
        return "pleasant"
    else:
        return "hot"

describe_weather(-4)  # "freezing"
describe_weather(15)  # "pleasant"  (15 is not < 15)
```

The second check doesn't need to say "and at least 0". If Python got that far, the first check already failed. **Order matters!**

### `<` or `<=`?

Most bugs in code like this live at the edges. Ask yourself what happens at *exactly* the boundary number, and pick `<` or `<=` to match.
