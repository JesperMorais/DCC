def format_duration(minutes: int) -> str:
    hours = minutes // 60
    rest = minutes % 60
    return f"{hours}:{rest:02d}"
