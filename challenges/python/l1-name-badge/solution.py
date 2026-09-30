def name_badge(first: str, last: str) -> str:
    first = first.strip().upper()
    last = last.strip()
    if last == "":
        return first
    return f"{first} {last[0].upper()}."
