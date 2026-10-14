from dataclasses import dataclass


@dataclass
class CartLine:
    name: str
    price_cents: int
    quantity: int


class GroceryCart:
    lines: list[CartLine] = []

    def __init__(self, customer: str) -> None:
        self.customer = customer

    def add(self, name: str, price_cents: int, quantity: int = 1) -> None:
        if quantity < 1:
            raise ValueError("quantity must be at least 1")
        for line in self.lines:
            if line.name == name:
                line.quantity += quantity
                return
        self.lines.append(CartLine(name, price_cents, quantity))

    def item_count(self) -> int:
        return sum(line.quantity for line in self.lines)

    def total_cents(self) -> int:
        return sum(line.price_cents * line.quantity for line in self.lines)
