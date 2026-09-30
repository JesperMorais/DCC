class ConfigError(Exception):
    """Base class for every configuration problem."""


class MissingKeyError(ConfigError):
    def __init__(self, key: str) -> None:
        super().__init__(f"missing key: {key}")
        self.key = key


def read_port(config: dict[str, str]) -> int:
    try:
        raw = config["port"]
    except KeyError:
        raise MissingKeyError("port") from None
    try:
        port = int(raw)
    except ValueError:
        raise ConfigError(f"port is not a number: {raw!r}") from None
    if not 1 <= port <= 65535:
        raise ConfigError(f"port out of range: {port}")
    return port


def collect_ports(configs: list[dict[str, str]]) -> tuple[list[int], list[str]]:
    ports: list[int] = []
    errors: list[str] = []
    for config in configs:
        try:
            port = read_port(config)
        except ConfigError as err:
            errors.append(str(err))
        else:
            ports.append(port)
    return ports, errors
