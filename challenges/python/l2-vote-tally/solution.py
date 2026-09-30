def tally_votes(votes: list[str]) -> dict[str, int]:
    counts: dict[str, int] = {}
    for vote in votes:
        if vote == "":
            continue
        counts[vote] = counts.get(vote, 0) + 1
    return counts
