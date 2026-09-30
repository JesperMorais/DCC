def add_guest(guests: list[str], name: str, capacity: int) -> str:
    if name in guests:
        return "already invited"
    if len(guests) >= capacity:
        return "full"
    guests.append(name)
    return "added"
