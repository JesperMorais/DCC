def test_parses_key_value_pairs():
    assert parse_config("host=db.local; port=5432; sslmode=require") == {
        "host": "db.local",
        "port": "5432",
        "sslmode": "require",
    }

def test_ignores_empty_segments():
    assert parse_config("host=db.local; port=5432;") == {"host": "db.local", "port": "5432"}
    assert parse_config(";; host=a ;  ; ") == {"host": "a"}

def test_strips_whitespace_and_lowercases_keys():
    assert parse_config("  User =  admin  ;SSLMode=require") == {"user": "admin", "sslmode": "require"}

def test_value_may_contain_equals_signs():
    assert parse_config("password=s3cr=t==") == {"password": "s3cr=t=="}

def test_later_duplicate_wins():
    assert parse_config("HOST=a; host=b") == {"host": "b"}

def test_segment_without_equals_is_an_error():
    with raises(ValueError, match="oops"):
        parse_config("host=db; oops ;")

def test_empty_input():
    assert parse_config("") == {}
    assert parse_config("   ") == {}
