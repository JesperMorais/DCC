from enum import Enum


class OrderStatus(Enum):
    PENDING = "pending"
    # add the other statuses here

    def advance(self) -> "OrderStatus":
        raise NotImplementedError


def parse_status(raw: str) -> OrderStatus:
    raise NotImplementedError
