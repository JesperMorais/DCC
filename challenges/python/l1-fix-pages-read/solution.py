def total_pages_read(days: int) -> int:
    total = 0
    for day in range(1, days + 1):
        total += day
    return total
