def active_subscribers(signups: list[str], unsubscribes: list[str]) -> list[str]:
    unsubscribed = {email.strip() for email in unsubscribes}
    seen: set[str] = set()
    active: list[str] = []
    for email in signups:
        key = email.strip().lower()
        if key in unsubscribed or key in seen:
            continue
        seen.add(key)
        active.append(email.strip())
    return active
