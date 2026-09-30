def price_label(item: str, unit_price: float, quantity: int) -> str:
    total = unit_price * quantity
    return f"{quantity} x {item} = {total:.2f}"
