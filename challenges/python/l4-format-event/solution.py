def format_event(name: str, *tags: str, **fields: object) -> str:
    parts = [name]
    parts += [f"#{tag}" for tag in tags]
    parts += [f"{key}={value}" for key, value in fields.items()]
    return " ".join(parts)


def format_error(*tags: str, **fields: object) -> str:
    return format_event("error", *tags, **fields)
