def average_rating(total_stars: int, review_count: int) -> float:
    if review_count == 0:
        return 0.0
    return total_stars / review_count
