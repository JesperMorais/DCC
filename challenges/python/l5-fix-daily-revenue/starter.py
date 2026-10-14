from dataclasses import dataclass
from datetime import date, datetime
from itertools import groupby


@dataclass(frozen=True)
class Order:
    order_id: str
    placed_at: datetime
    amount_cents: int


def order_day(order: Order) -> date:
    return order.placed_at.date()


def daily_revenue(orders: list[Order]) -> dict[date, int]:
    return {
        day: sum(o.amount_cents for o in day_orders)
        for day, day_orders in groupby(orders, key=order_day)
    }
