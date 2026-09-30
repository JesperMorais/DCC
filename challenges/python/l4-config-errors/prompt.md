A deploy tool reads each service's config (a `dict[str, str]`) and needs the port. Callers want to catch **one** error type for "bad config" but still be able to tell a missing key apart. Build the exceptions and two functions.

**Exceptions** (stubs are in the starter):

- `ConfigError(Exception)` is the base class for every config problem.
- `MissingKeyError(ConfigError)` is created as `MissingKeyError(key)`. It has a `.key` attribute, and its message is `"missing key: <key>"`.

**`read_port(config)`** returns `config["port"]` as an `int`:

| Situation | Raise | Message |
|---|---|---|
| no `"port"` key | `MissingKeyError` | `missing key: port` |
| not an integer | `ConfigError` | `port is not a number: '<raw>'` |
| outside `1`–`65535` | `ConfigError` | `port out of range: <port>` |

Surrounding whitespace is fine (`" 8080 "` is `8080`).

**`collect_ports(configs)`** reads every config and returns `(ports, errors)`: the good ports, and `str(error)` for every config that failed, both in input order.

```python
read_port({"port": "8080"})   # 8080
read_port({})                 # MissingKeyError: missing key: port
read_port({"port": "http"})   # ConfigError: port is not a number: 'http'
collect_ports([{"port": "80"}, {}, {"port": "0"}])
# ([80], ["missing key: port", "port out of range: 0"])
```
