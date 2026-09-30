class FakeApi:
    def __init__(self) -> None:
        self.calls: list[str] = []
        self.down = False

    def fetch(self, address: str) -> tuple[float, float]:
        self.calls.append(address)
        if self.down:
            raise ConnectionError("geocoder unavailable")
        return (float(len(address)), 18.0)


def test_repeat_lookups_hit_the_cache():
    api = FakeApi()
    geo = make_geocoder(api.fetch, maxsize=10)
    assert geo("Drottninggatan 1") == (16.0, 18.0)
    assert geo("Drottninggatan 1") == (16.0, 18.0)
    assert api.calls == ["drottninggatan 1"]


def test_addresses_are_normalised_before_caching():
    api = FakeApi()
    geo = make_geocoder(api.fetch, maxsize=10)
    geo("Main St")
    geo("  MAIN ST ")
    geo("main st")
    assert api.calls == ["main st"]


def test_evicts_the_least_recently_used_address():
    api = FakeApi()
    geo = make_geocoder(api.fetch, maxsize=2)
    geo("a")
    geo("b")
    geo("a")  # hit: "a" is now the most recently used
    geo("c")  # full, so evicts "b"
    geo("a")  # still cached
    assert api.calls == ["a", "b", "c"]
    geo("b")  # was evicted, fetched again
    assert api.calls == ["a", "b", "c", "b"]


def test_failures_are_not_cached():
    api = FakeApi()
    geo = make_geocoder(api.fetch, maxsize=10)
    api.down = True
    with raises(ConnectionError):
        geo("x")
    api.down = False
    assert geo("x") == (1.0, 18.0)
    assert api.calls == ["x", "x"]


def test_each_geocoder_has_its_own_cache():
    api = FakeApi()
    a = make_geocoder(api.fetch, maxsize=10)
    b = make_geocoder(api.fetch, maxsize=10)
    a("home")
    b("home")
    assert api.calls == ["home", "home"]
