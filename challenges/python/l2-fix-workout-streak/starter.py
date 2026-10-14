def longest_streak(days: list[bool]) -> int:
    best = 0
    for worked_out in days:
        current = 0
        if worked_out:
            current += 1
            best = max(best, current)
        else:
            current = 0
    return best
