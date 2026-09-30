def clean_tags(tags: list[str]) -> list[str]:
    return [tag.strip().lower() for tag in tags if tag.strip()]
