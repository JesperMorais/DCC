from dataclasses import dataclass, field


@dataclass
class Playlist:
    name: str
    # add the tracks field here

    def add(self, seconds: int) -> None:
        raise NotImplementedError

    def duration(self) -> str:
        raise NotImplementedError
