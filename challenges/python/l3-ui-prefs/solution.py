from typing import Any


def ui_prefs(profile: dict[str, Any]) -> tuple[str, int]:
    settings: dict[str, Any] = profile.get("settings") or {}
    ui: dict[str, Any] = settings.get("ui") or {}
    theme: str = ui.get("theme", "light")
    font_size: int = ui.get("font_size", 14)
    return theme, font_size
