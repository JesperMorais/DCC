`asyncio.gather(*jobs)` runs **everything** at once. That's fine for 5 downloads and a disaster for 5 000 (rate limits, open sockets, memory). Write **`gather_with_limit`**, which works like `gather` but never runs more than `limit` jobs at the same time.

```python
async def gather_with_limit[T](aws: Iterable[Awaitable[T]], limit: int) -> list[T]
```

- `aws` are awaitables, typically coroutine objects like `download(url)` that haven't started yet.
- At most `limit` are running at any moment, and as soon as one finishes, the next one starts (no waiting for a whole "batch").
- Return the results **in input order**, regardless of which job finished first.
- If a job raises, `gather_with_limit` raises that exception.
- Empty input returns `[]`. `limit < 1` raises `ValueError`.

```python
async def download(url: str) -> bytes: ...

pages = await gather_with_limit((download(u) for u in urls), limit=10)
```
