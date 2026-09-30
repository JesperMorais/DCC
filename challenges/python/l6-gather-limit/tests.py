import asyncio


class Tracker:
    def __init__(self) -> None:
        self.running = 0
        self.peak = 0
        self.started = 0

    async def job(self, seconds: float, value):
        self.started += 1
        self.running += 1
        self.peak = max(self.peak, self.running)
        try:
            await asyncio.sleep(seconds)
        finally:
            self.running -= 1
        return value


async def test_returns_results_in_input_order():
    t = Tracker()
    delays = [0.03, 0.005, 0.015, 0.001]
    out = await gather_with_limit([t.job(d, f"done {d}") for d in delays], limit=2)
    assert out == ["done 0.03", "done 0.005", "done 0.015", "done 0.001"]


async def test_never_exceeds_the_limit():
    t = Tracker()
    out = await gather_with_limit((t.job(0.005, n) for n in range(7)), limit=3)
    assert out == list(range(7))
    assert t.peak == 3
    assert t.started == 7


async def test_starts_the_next_job_as_soon_as_a_slot_frees_up():
    t = Tracker()
    pending = asyncio.create_task(
        gather_with_limit([t.job(d, d) for d in [0.06, 0.005, 0.005, 0.005]], limit=2)
    )
    await asyncio.sleep(0.035)
    started_while_the_slow_job_runs = t.started
    await pending
    assert started_while_the_slow_job_runs == 4


async def test_a_limit_above_the_job_count_is_fine():
    t = Tracker()
    assert await gather_with_limit([t.job(0.002, "a"), t.job(0.002, "b")], limit=10) == ["a", "b"]
    assert t.peak == 2


async def test_errors_propagate():
    async def boom():
        await asyncio.sleep(0.001)
        raise LookupError("job 2 failed")

    t = Tracker()
    with raises(LookupError, match="job 2 failed"):
        await gather_with_limit([t.job(0.001, 1), boom(), t.job(0.001, 3)], limit=2)


async def test_empty_input_and_bad_limit():
    assert await gather_with_limit([], limit=3) == []
    with raises(ValueError):
        await gather_with_limit([], limit=0)
