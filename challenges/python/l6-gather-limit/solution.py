import asyncio
from collections.abc import Awaitable, Iterable


async def gather_with_limit[T](aws: Iterable[Awaitable[T]], limit: int) -> list[T]:
    if limit < 1:
        raise ValueError("limit must be at least 1")
    sem = asyncio.Semaphore(limit)

    async def run(aw: Awaitable[T]) -> T:
        async with sem:
            return await aw

    return await asyncio.gather(*(run(aw) for aw in aws))
