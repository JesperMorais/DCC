class FakeClock:
    def __init__(self, now: float = 0.0) -> None:
        self.now = now
        self.reads = 0

    def __call__(self) -> float:
        self.reads += 1
        return self.now


def test_allows_up_to_the_limit_then_refuses():
    allow = make_rate_limiter(3, window=1.0, clock=FakeClock())
    assert [allow() for _ in range(5)] == [True, True, True, False, False]


def test_a_new_window_restores_the_budget():
    clock = FakeClock()
    allow = make_rate_limiter(2, window=1.0, clock=clock)
    assert [allow(), allow(), allow()] == [True, True, False]
    clock.now = 0.999
    assert allow() is False
    clock.now = 1.0  # exactly one window later: fresh window
    assert [allow(), allow(), allow()] == [True, True, False]


def test_the_window_starts_at_the_first_call_not_at_creation():
    clock = FakeClock(now=100.0)
    allow = make_rate_limiter(1, window=10.0, clock=clock)
    assert clock.reads == 0
    clock.now = 105.0
    assert allow() is True
    clock.now = 114.0  # only 9 s after the window opened
    assert allow() is False
    clock.now = 115.0
    assert allow() is True


def test_after_a_long_pause_the_window_reopens_at_the_current_time():
    clock = FakeClock()
    allow = make_rate_limiter(1, window=1.0, clock=clock)
    allow()
    clock.now = 7.5
    assert allow() is True
    clock.now = 8.0  # 0.5 s into the window opened at 7.5
    assert allow() is False


def test_limiters_are_independent():
    clock = FakeClock()
    a = make_rate_limiter(1, window=1.0, clock=clock)
    b = make_rate_limiter(1, window=1.0, clock=clock)
    assert a() is True
    assert a() is False
    assert b() is True
