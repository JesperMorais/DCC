def paginate(items: list[str], page: int, per_page: int) -> list[str]:
    if page < 1:
        raise ValueError("page must be at least 1")
    if per_page < 1:
        raise ValueError("per_page must be at least 1")
    start = (page - 1) * per_page
    return items[start:start + per_page]
