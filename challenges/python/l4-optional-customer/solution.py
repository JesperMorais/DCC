from dataclasses import dataclass


@dataclass
class Customer:
    email: str
    name: str
    nickname: str | None = None


def _normalise(email: str) -> str:
    return email.strip().lower()


def find_customer(customers: list[Customer], email: str) -> Customer | None:
    wanted = _normalise(email)
    for customer in customers:
        if _normalise(customer.email) == wanted:
            return customer
    return None


def greeting_name(customers: list[Customer], email: str) -> str:
    customer = find_customer(customers, email)
    if customer is None:
        return "Guest"
    return customer.nickname or customer.name
