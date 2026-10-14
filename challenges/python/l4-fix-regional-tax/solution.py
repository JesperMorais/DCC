from collections.abc import Callable

type PriceFn = Callable[[int], int]


def make_tax_calculators(rates: dict[str, float]) -> dict[str, PriceFn]:
    calculators: dict[str, PriceFn] = {}
    for region, rate in rates.items():
        if not 0 <= rate < 1:
            raise ValueError(f"bad tax rate for {region}: {rate}")

        def with_tax(net_cents: int, rate: float = rate) -> int:
            return round(net_cents * (1 + rate))

        calculators[region] = with_tax
    return calculators


def gross_total(lines: list[tuple[str, int]], calculators: dict[str, PriceFn]) -> int:
    return sum(calculators[region](net) for region, net in lines)
