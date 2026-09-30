from dataclasses import dataclass, field


@dataclass
class Playlist:
    name: str
    tracks: list[int] = field(default_factory=list)

    def add(self, seconds: int) -> None:
        self.tracks.append(seconds)

    def duration(self) -> str:
        minutes, seconds = divmod(sum(self.tracks), 60)
        return f"{minutes}:{seconds:02d}"
