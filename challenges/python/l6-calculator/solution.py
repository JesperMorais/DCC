import re

_TOKEN = re.compile(r"[0-9]+(?:\.[0-9]+)?|\S")
_NUMBER = re.compile(r"[0-9]+(?:\.[0-9]+)?")
_SYMBOLS = frozenset("+-*/()")


def tokenize(expr: str) -> list[str]:
    tokens: list[str] = _TOKEN.findall(expr)
    for token in tokens:
        if token not in _SYMBOLS and not _NUMBER.fullmatch(token):
            raise ValueError(f"unexpected character {token!r}")
    return tokens


def calculate(expr: str) -> float:
    tokens = tokenize(expr)
    pos = 0

    def peek() -> str | None:
        return tokens[pos] if pos < len(tokens) else None

    def take() -> str:
        nonlocal pos
        if pos >= len(tokens):
            raise ValueError("unexpected end of expression")
        pos += 1
        return tokens[pos - 1]

    # expression := term (("+" | "-") term)*
    def expression() -> float:
        value = term()
        while peek() in ("+", "-"):
            op = take()
            rhs = term()
            value = value + rhs if op == "+" else value - rhs
        return value

    # term := factor (("*" | "/") factor)*
    def term() -> float:
        value = factor()
        while peek() in ("*", "/"):
            op = take()
            rhs = factor()
            value = value * rhs if op == "*" else value / rhs
        return value

    # factor := NUMBER | "(" expression ")"
    def factor() -> float:
        token = take()
        if _NUMBER.fullmatch(token):
            return float(token)
        if token == "(":
            value = expression()
            if take() != ")":
                raise ValueError("expected ')'")
            return value
        raise ValueError(f"unexpected token {token!r}")

    result = expression()
    if pos != len(tokens):
        raise ValueError(f"unexpected token {tokens[pos]!r}")
    return result
