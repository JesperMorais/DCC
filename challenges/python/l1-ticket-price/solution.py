def ticket_price(age: int) -> int:
    if age <= 2:
        return 0
    elif age <= 12:
        return 60
    elif age <= 64:
        return 120
    else:
        return 80
