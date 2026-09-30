def basket_total(basket: list[str], prices: dict[str, int]) -> int:
    total = 0
    for item in basket:
        total += prices.get(item, 0)
    return total
