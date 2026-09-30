from collections.abc import Iterator
from contextlib import contextmanager

_MISSING = object()


@contextmanager
def override(settings: dict[str, object], **changes: object) -> Iterator[dict[str, object]]:
    saved = {key: settings.get(key, _MISSING) for key in changes}
    settings.update(changes)
    try:
        yield settings
    finally:
        for key, old in saved.items():
            if old is _MISSING:
                settings.pop(key, None)
            else:
                settings[key] = old
