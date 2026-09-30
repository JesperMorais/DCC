from collections import Counter


def top_failing_endpoints(lines: list[str], n: int) -> list[tuple[str, int]]:
    failures: Counter[str] = Counter()
    for line in lines:
        method, path, status, _ = line.split()
        if int(status) >= 500:
            failures[f"{method} {path}"] += 1
    return failures.most_common(n)
