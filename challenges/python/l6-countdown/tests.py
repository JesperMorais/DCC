def test_counts_down_to_one():
    assert list(Countdown(3)) == [3, 2, 1]
    assert [n for n in Countdown(5) if n % 2] == [5, 3, 1]


def test_zero_yields_nothing_and_negative_is_rejected():
    assert list(Countdown(0)) == []
    with raises(ValueError):
        Countdown(-1)


def test_a_countdown_can_be_iterated_again_and_nested():
    c = Countdown(2)
    assert list(c) == [2, 1]
    assert list(c) == [2, 1]
    assert [(a, b) for a in c for b in c] == [(2, 2), (2, 1), (1, 2), (1, 1)]


def test_each_iter_call_gives_a_fresh_iterator():
    c = Countdown(3)
    first, second = iter(c), iter(c)
    assert isinstance(first, CountdownIterator)
    assert first is not second
    next(first)
    assert next(second) == 3


def test_the_iterator_tracks_what_is_left():
    it = iter(Countdown(4))
    assert iter(it) is it
    assert it.remaining == 4
    assert next(it) == 4
    assert it.remaining == 3
    assert list(it) == [3, 2, 1]
    assert it.remaining == 0


def test_an_exhausted_iterator_stays_exhausted():
    it = iter(Countdown(1))
    assert next(it) == 1
    with raises(StopIteration):
        next(it)
    with raises(StopIteration):
        next(it)
    assert next(it, "liftoff") == "liftoff"
