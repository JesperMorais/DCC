def test_name_only():
    assert format_event("deploy") == "deploy"

def test_any_number_of_tags():
    assert format_event("login", "web") == "login #web"
    assert format_event("login", "web", "mobile", "eu") == "login #web #mobile #eu"

def test_fields_keep_their_order_and_are_stringified():
    assert format_event("login", user="ada", ok=True, ms=12.5) == "login user=ada ok=True ms=12.5"

def test_tags_come_before_fields():
    assert format_event("login", "web", user="ada") == "login #web user=ada"

def test_format_error_forwards_everything():
    assert format_error("db", table="orders", retries=3) == "error #db table=orders retries=3"
    assert format_error() == "error"

def test_forwarding_unpacked_collections():
    tags = ["payments", "stripe"]
    fields = {"order": 42, "code": None}
    assert format_error(*tags, **fields) == "error #payments #stripe order=42 code=None"
