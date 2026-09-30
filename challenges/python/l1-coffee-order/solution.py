def order_summary(drink: str, size: str = "medium", shots: int = 1) -> str:
    word = "shot" if shots == 1 else "shots"
    return f"{size} {drink}, {shots} {word}"
