def edit_distance(a: str, b: str) -> int:
    # prev[j] = distance between the first i-1 chars of a and the first j chars of b
    prev = list(range(len(b) + 1))
    for i, ca in enumerate(a, start=1):
        cur = [i]
        for j, cb in enumerate(b, start=1):
            cur.append(
                min(
                    prev[j] + 1,  # delete ca
                    cur[j - 1] + 1,  # insert cb
                    prev[j - 1] + (ca != cb),  # substitute (free if equal)
                )
            )
        prev = cur
    return prev[-1]
