def leaderboard(results: list[tuple[str, int, float]]) -> list[str]:
    ranked = sorted(results, key=lambda r: (-r[1], r[2], r[0]))
    return [
        f"{rank}. {name} ({score} pts)"
        for rank, (name, score, _) in enumerate(ranked, start=1)
    ]
