import re

_TOKEN = re.compile(r"[0-9]+(?:\.[0-9]+)?|\S")
_NUMBER = re.compile(r"[0-9]+(?:\.[0-9]+)?")
_SYMBOLS = frozenset("+-*/()")

def tokenize(expr: str) -> list[str]:
    """Provided: splits `expr` into number, operator and paren tokens."""
    tokens: list[str] = _TOKEN.findall(expr)
    for token in tokens:
        if token not in _SYMBOLS and not _NUMBER.fullmatch(token):
            raise ValueError(f"unexpected character {token!r}")
    return tokens

def calculate(expr: str) -> float:
    tokens = tokenize(expr)
    pos = 0

    def peek() -> str | None:
        """The current token, or None at the end. Doesn't consume it."""
        return tokens[pos] if pos < len(tokens) else None

    def take() -> str:
        """Consume and return the current token. ValueError at the end."""
        nonlocal pos
        if pos >= len(tokens):
            raise ValueError("unexpected end of expression")
        pos += 1
        return tokens[pos - 1]

    # Write one function per grammar rule here, then call the top one.
    raise NotImplementedError
