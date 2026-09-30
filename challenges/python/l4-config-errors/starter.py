class ConfigError(Exception):
    pass


class MissingKeyError(ConfigError):
    pass  # store the key and build the message


def read_port(config: dict[str, str]) -> int:
    raise NotImplementedError


def collect_ports(configs: list[dict[str, str]]) -> tuple[list[int], list[str]]:
    return [], []
