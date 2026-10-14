import asyncio
from collections.abc import Awaitable, Callable
from dataclasses import dataclass

type PriceSource = Callable[[str], Awaitable[int]]


@dataclass(frozen=True)
class Quote:
    sku: str
    price_cents: int
    supplier: str


class PriceAggregator:
    def __init__(self, suppliers: dict[str, PriceSource], timeout: float = 0.5) -> None:
        if not suppliers:
            raise ValueError("need at least one supplier")
        self._suppliers = suppliers
        self._timeout = timeout

    async def _ask(self, supplier: str, sku: str) -> Quote | None:
        try:
            price = await asyncio.wait_for(self._suppliers[supplier](sku), self._timeout)
        except (TimeoutError, LookupError):
            return None
        return Quote(sku, price, supplier)

    async def best_quote(self, sku: str) -> Quote | None:
        answers = await asyncio.gather(*(self._ask(name, sku) for name in self._suppliers))
        quotes = [q for q in answers if q is not None]
        return min(quotes, key=lambda q: q.price_cents, default=None)

    async def best_quotes(self, skus: list[str]) -> dict[str, Quote | None]:
        tasks = [asyncio.create_task(self.best_quote(sku)) for sku in skus]
        results = [await t for t in asyncio.as_completed(tasks)]
        return dict(zip(skus, results))

    async def basket_total(self, skus: list[str]) -> int:
        quotes = await self.best_quotes(skus)
        unavailable = sorted(sku for sku, q in quotes.items() if q is None)
        if unavailable:
            raise LookupError(f"no supplier has: {', '.join(unavailable)}")
        return sum(q.price_cents for q in quotes.values() if q is not None)
