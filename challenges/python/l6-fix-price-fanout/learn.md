### Running coroutines concurrently, and getting the results back

`asyncio.create_task(coro)` starts a coroutine running in the background. There are two common ways to collect many results:

```python
tasks = [asyncio.create_task(fetch(url)) for url in urls]

# 1. gather: wait for all, results in the order you passed them
pages = await asyncio.gather(*tasks)

# 2. as_completed: get each result as soon as it's ready
for next_done in asyncio.as_completed(tasks):
    page = await next_done
    show_progress(page)
```

Both run the tasks concurrently, so total time is the slowest task, not the sum. The difference is **ordering**: `gather` preserves *input* order. `as_completed` yields in *finishing* order, which depends on timing and changes from run to run. It's great for progress bars and "first answer wins", but the result alone doesn't tell you which input it belongs to.

### Debugging technique: make timing visible, then vary it

Bugs that depend on timing hide in tests where everything takes equally long. Give each fake call a **different** delay (fast, slow, medium), log when each one starts and finishes, and compare against the order your code assumes. If the output changes when only the delays change, some step is relying on timing for correctness.
