def word_index(lines: list[str]) -> dict[str, list[int]]:
    index: dict[str, list[int]] = {}
    for number, line in enumerate(lines, start=1):
        for word in line.lower().split():
            index.get(word, []).append(number)
    return index
