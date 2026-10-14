from datetime import date, datetime


def order(order_id, timestamp, amount_cents):
    return Order(order_id, datetime.fromisoformat(timestamp), amount_cents)


def test_sums_each_day():
    orders = [
        order("A1", "2026-10-12 09:15", 12_000),
        order("A2", "2026-10-12 17:40", 3_500),
        order("A3", "2026-10-13 11:05", 8_000),
    ]
    assert daily_revenue(orders) == {date(2026, 10, 12): 15_500, date(2026, 10, 13): 8_000}


def test_a_single_order():
    assert daily_revenue([order("B1", "2026-10-14 23:59", 990)]) == {date(2026, 10, 14): 990}


def test_no_orders():
    assert daily_revenue([]) == {}


def test_orders_out_of_order_are_still_counted():
    orders = [
        order("C1", "2026-10-12 10:00", 1_000),
        order("C2", "2026-10-13 08:30", 2_000),
        order("C3", "2026-10-12 21:10", 4_000),
        order("C4", "2026-10-13 12:00", 500),
    ]
    assert daily_revenue(orders) == {date(2026, 10, 12): 5_000, date(2026, 10, 13): 2_500}


def test_total_matches_the_sum_of_all_orders():
    orders = [
        order("D1", "2026-10-11 13:00", 700),
        order("D2", "2026-10-10 13:00", 300),
        order("D3", "2026-10-11 09:00", 1_100),
        order("D4", "2026-10-10 18:00", 900),
        order("D5", "2026-10-11 20:00", 50),
    ]
    totals = daily_revenue(orders)
    assert sum(totals.values()) == 3_050
    assert totals[date(2026, 10, 10)] == 1_200
