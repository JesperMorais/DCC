from collections import defaultdict


def group_by_level(lines: list[str]) -> dict[str, list[str]]:
    groups: defaultdict[str, list[str]] = defaultdict(list)
    for line in lines:
        level, sep, message = line.partition(":")
        if not sep:
            continue
        groups[level.strip().upper()].append(message.strip())
    return dict(groups)
