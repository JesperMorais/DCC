def access_diff(before: list[str], after: list[str]) -> dict[str, list[str]]:
    old, new = set(before), set(after)
    return {
        "added": sorted(new - old),
        "removed": sorted(old - new),
        "kept": sorted(old & new),
    }
