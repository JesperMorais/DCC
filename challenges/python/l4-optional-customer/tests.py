ADA = Customer("ada@example.com", "Ada Lovelace", "Ada")
LINUS = Customer("linus@example.com", "Linus Torvalds")
GRACE = Customer(" Grace@Example.com ", "Grace Hopper", "")
CUSTOMERS = [ADA, LINUS, GRACE]

def test_finds_the_customer_object():
    assert find_customer(CUSTOMERS, "linus@example.com") is LINUS

def test_matching_ignores_case_and_whitespace():
    assert find_customer(CUSTOMERS, " ADA@example.com") is ADA
    assert find_customer(CUSTOMERS, "grace@example.com") is GRACE

def test_no_match_gives_none():
    assert find_customer(CUSTOMERS, "bob@example.com") is None
    assert find_customer([], "ada@example.com") is None

def test_greets_by_nickname_when_set():
    assert greeting_name(CUSTOMERS, "ada@example.com") == "Ada"

def test_falls_back_to_the_full_name():
    assert greeting_name(CUSTOMERS, "linus@example.com") == "Linus Torvalds"
    assert greeting_name(CUSTOMERS, "GRACE@example.com") == "Grace Hopper"

def test_unknown_email_is_a_guest():
    assert greeting_name(CUSTOMERS, "bob@example.com") == "Guest"
