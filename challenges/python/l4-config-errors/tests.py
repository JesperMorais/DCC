def test_exception_hierarchy():
    assert issubclass(ConfigError, Exception)
    assert issubclass(MissingKeyError, ConfigError)
    err = MissingKeyError("host")
    assert err.key == "host"
    assert str(err) == "missing key: host"

def test_reads_a_valid_port():
    assert read_port({"port": "8080"}) == 8080
    assert read_port({"port": " 443 ", "host": "x"}) == 443

def test_missing_port_raises_missing_key_error():
    with raises(MissingKeyError, match="missing key: port"):
        read_port({"host": "db"})
    with raises(MissingKeyError) as caught:
        read_port({})
    assert caught.value.key == "port"

def test_non_number_is_a_config_error_but_not_a_missing_key():
    try:
        read_port({"port": "http"})
    except MissingKeyError:
        assert False, "a bad value is not a missing key"
    except ConfigError as err:
        assert str(err) == "port is not a number: 'http'"
    else:
        assert False, "expected ConfigError"

def test_range_is_checked_at_both_ends():
    assert read_port({"port": "1"}) == 1
    assert read_port({"port": "65535"}) == 65535
    with raises(ConfigError, match="port out of range: 0"):
        read_port({"port": "0"})
    with raises(ConfigError, match="port out of range: 65536"):
        read_port({"port": "65536"})

def test_collect_ports_splits_good_and_bad():
    configs = [{"port": "80"}, {}, {"port": "0"}, {"port": "abc"}, {"port": "5432"}]
    assert collect_ports(configs) == (
        [80, 5432],
        ["missing key: port", "port out of range: 0", "port is not a number: 'abc'"],
    )
    assert collect_ports([]) == ([], [])
