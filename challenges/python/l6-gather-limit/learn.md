### Coroutines are lazy

Calling an `async def` function doesn't run it. You get a **coroutine object**, which sits there until something awaits it (or wraps it in a task). This is unlike JavaScript promises, which start immediately:

```python
async def fetch(n: int) -> int:
    print("fetching", n)
    await asyncio.sleep(0.01)
    return n

coro = fetch(1)        # nothing printed yet
await coro             # "fetching 1", returns 1
```

So a list of coroutines is a list of *jobs that haven't started*. You decide when each one begins, by deciding when to await it.

### `asyncio.gather`

`await asyncio.gather(a, b, c)` runs its arguments **concurrently** and returns their results as a list **in argument order**. If one raises, `gather` raises that exception.

### `asyncio.Semaphore`: a pool of permits

A semaphore holds `n` permits. `async with sem:` takes one, waiting if none are free, and gives it back when the block exits, even on an exception:

```python
sem = asyncio.Semaphore(2)

async def use_printer(doc: str) -> None:
    async with sem:              # at most 2 coroutines in here at once
        await print_page(doc)
```

A common pattern is to **wrap** each job in a small coroutine that takes a permit before awaiting the job. Then hand all the wrappers to `gather`. They all *start*, but most of them immediately wait at the semaphore.

### No locks needed for counters

asyncio runs on one thread, and control only switches at an `await`. So `running += 1` between two awaits can't race.
