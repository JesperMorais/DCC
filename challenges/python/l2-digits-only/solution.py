def digits_only(phone: str) -> str:
    result = ""
    for ch in phone:
        if ch.isdigit():
            result += ch
    return result
