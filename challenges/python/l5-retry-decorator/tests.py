def make_flaky(failures: int, exc: type[Exception] = ConnectionError):
    """Returns (fn, calls): fn raises `exc` on its first `failures` calls."""
    calls: list[tuple] = []

    def fetch(user_id: int, *, fields: str = "name") -> str:
        """Fetch a user from the API."""
        calls.append((user_id, fields))
        if len(calls) <= failures:
            raise exc(f"attempt {len(calls)} failed")
        return f"user {user_id} ({fields})"

    return fetch, calls


def test_succeeds_after_transient_failures():
    fetch, calls = make_flaky(failures=2)
    assert retry(times=3)(fetch)(42) == "user 42 (name)"
    assert len(calls) == 3


def test_does_not_call_again_after_a_success():
    fetch, calls = make_flaky(failures=0)
    retry(times=5)(fetch)(1)
    assert len(calls) == 1


def test_reraises_the_last_error_when_out_of_attempts():
    fetch, calls = make_flaky(failures=10)
    with raises(ConnectionError, match="attempt 3 failed"):
        retry(times=3)(fetch)(1)
    assert len(calls) == 3


def test_other_exceptions_are_not_retried():
    fetch, calls = make_flaky(failures=10, exc=KeyError)
    with raises(KeyError):
        retry(times=3, on=(ConnectionError, TimeoutError))(fetch)(1)
    assert len(calls) == 1


def test_passes_arguments_through_on_every_attempt():
    fetch, calls = make_flaky(failures=1)
    assert retry(times=2)(fetch)(7, fields="email") == "user 7 (email)"
    assert calls == [(7, "email"), (7, "email")]


def test_keeps_name_and_docstring():
    @retry(times=2)
    def fetch_user(user_id: int) -> str:
        """Fetch a user from the API."""
        return str(user_id)

    assert fetch_user.__name__ == "fetch_user"
    assert fetch_user.__doc__ == "Fetch a user from the API."


def test_times_below_one_is_rejected_at_decoration_time():
    with raises(ValueError):
        retry(times=0)
