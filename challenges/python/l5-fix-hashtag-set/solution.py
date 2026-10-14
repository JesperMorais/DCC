from collections import Counter
from collections.abc import Iterable


class Hashtag:
    def __init__(self, text: str) -> None:
        self.text = text.removeprefix("#").strip()
        if not self.text:
            raise ValueError("empty hashtag")

    def __repr__(self) -> str:
        return f"Hashtag('#{self.text}')"

    def __eq__(self, other: object) -> bool:
        if not isinstance(other, Hashtag):
            return NotImplemented
        return self.text.casefold() == other.text.casefold()

    def __hash__(self) -> int:
        return hash(self.text.casefold())


def unique_hashtags(tags: Iterable[str]) -> set[Hashtag]:
    return {Hashtag(t) for t in tags}


def trending(posts: list[list[str]], top: int) -> list[tuple[str, int]]:
    counts = Counter(Hashtag(t) for post in posts for t in post)
    return [(tag.text, n) for tag, n in counts.most_common(top)]
