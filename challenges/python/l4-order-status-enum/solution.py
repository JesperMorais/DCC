from enum import Enum


class OrderStatus(Enum):
    PENDING = "pending"
    PAID = "paid"
    SHIPPED = "shipped"
    DELIVERED = "delivered"

    def advance(self) -> "OrderStatus":
        members = list(OrderStatus)
        index = members.index(self)
        if index == len(members) - 1:
            raise ValueError(f"{self.value} is final")
        return members[index + 1]


def parse_status(raw: str) -> OrderStatus:
    return OrderStatus(raw.strip().lower())
