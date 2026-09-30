def test_uses_both_defaults():
    assert order_summary("latte") == "medium latte, 1 shot"


def test_everything_given():
    assert order_summary("mocha", "large", 2) == "large mocha, 2 shots"


def test_only_size_given():
    assert order_summary("flat white", "small") == "small flat white, 1 shot"


def test_shots_as_a_keyword_argument():
    assert order_summary("americano", shots=0) == "medium americano, 0 shots"


def test_plural_for_many_shots():
    assert order_summary("cortado", size="large", shots=3) == "large cortado, 3 shots"
