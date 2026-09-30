def times_table(n: int, count: int) -> list[int]:
    result: list[int] = []
    for i in range(1, count + 1):
        result.append(n * i)
    return result
