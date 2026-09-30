def parse_config(text: str) -> dict[str, str]:
    config: dict[str, str] = {}
    for raw in text.split(";"):
        segment = raw.strip()
        if not segment:
            continue
        if "=" not in segment:
            raise ValueError(f"malformed segment: {segment!r}")
        key, value = segment.split("=", 1)
        config[key.strip().lower()] = value.strip()
    return config
