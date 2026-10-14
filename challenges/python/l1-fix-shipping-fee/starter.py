def shipping_fee(order_total: float) -> float:
    if order_total >= 20.0:
        return 4.99
    elif order_total >= 50.0:
        return 0.0
    else:
        return 7.99
