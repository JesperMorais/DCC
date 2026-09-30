### Your own exception classes

Subclass `Exception` (or one of your own exceptions) to create an error type that callers can catch **by name**:

```python
class PaymentError(Exception):
    """Base class: anything that went wrong while charging."""

class CardDeclined(PaymentError):
    def __init__(self, reason: str) -> None:
        super().__init__(f"card declined: {reason}")  # sets the message
        self.reason = reason                          # extra data for callers
```

`except PaymentError:` catches `CardDeclined` too, because it's a subclass. A hierarchy lets callers decide how specific to be.

### Translating low-level errors

Don't let a raw `KeyError` leak out of a config loader. Catch it and raise something meaningful. `from None` hides the original traceback, which is just noise here:

```python
try:
    token = env["TOKEN"]
except KeyError:
    raise PaymentError("no API token configured") from None
```

### `try` / `except` / `else`

The `else` block runs only if the `try` block raised **nothing**. Keep the `try` small, so it only wraps the call that can fail, and put the success path in `else`:

```python
try:
    receipt = charge(card)
except PaymentError as err:
    failed.append(str(err))
else:
    sent.append(receipt)
```
