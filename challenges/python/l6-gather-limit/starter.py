import asyncio
from collections.abc import Awaitable, Iterable


async def gather_with_limit[T](aws: Iterable[Awaitable[T]], limit: int) -> list[T]:
    raise NotImplementedError
