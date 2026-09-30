from collections.abc import Mapping


def flatten_config(config: Mapping[str, object], sep: str = ".") -> dict[str, object]:
    out: dict[str, object] = {}

    def walk(node: Mapping[str, object], prefix: str) -> None:
        for key, value in node.items():
            full_key = f"{prefix}{sep}{key}" if prefix else key
            if isinstance(value, dict):
                walk(value, full_key)
            else:
                out[full_key] = value

    walk(config, "")
    return out
