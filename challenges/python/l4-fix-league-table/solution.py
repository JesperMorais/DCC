from dataclasses import dataclass


@dataclass(frozen=True)
class TeamRecord:
    name: str
    won: int
    drawn: int
    lost: int
    goals_for: int
    goals_against: int

    @property
    def points(self) -> int:
        return 3 * self.won + self.drawn

    @property
    def goal_difference(self) -> int:
        return self.goals_for - self.goals_against


def league_table(teams: list[TeamRecord]) -> list[str]:
    ranked = sorted(teams, key=lambda t: (-t.points, -t.goal_difference, t.name))
    return [f"{pos}. {t.name} {t.points}" for pos, t in enumerate(ranked, start=1)]
