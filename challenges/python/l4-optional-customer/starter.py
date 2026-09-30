from dataclasses import dataclass


@dataclass
class Customer:
    email: str
    name: str
    nickname: str | None = None


def find_customer(customers: list[Customer], email: str) -> Customer | None:
    return None


def greeting_name(customers: list[Customer], email: str) -> str:
    return ""
