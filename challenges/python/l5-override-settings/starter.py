from collections.abc import Iterator
from contextlib import contextmanager


@contextmanager
def override(settings: dict[str, object], **changes: object) -> Iterator[dict[str, object]]:
    yield settings
