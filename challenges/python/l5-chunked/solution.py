from collections.abc import Iterable, Iterator
from itertools import islice


def chunked[T](iterable: Iterable[T], n: int) -> Iterator[list[T]]:
    if n < 1:
        raise ValueError("n must be at least 1")
    it = iter(iterable)
    while chunk := list(islice(it, n)):
        yield chunk
