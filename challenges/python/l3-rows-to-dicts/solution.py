def rows_to_dicts(header: list[str], rows: list[list[str]]) -> list[dict[str, str]]:
    keys = [name.strip().lower() for name in header]
    return [
        {key: value.strip() for key, value in zip(keys, row)}
        for row in rows
        if len(row) == len(keys)
    ]
