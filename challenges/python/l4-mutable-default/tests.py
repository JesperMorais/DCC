def test_without_a_list_returns_a_new_one():
    assert add_tag("urgent") == ["urgent"]

def test_calls_do_not_share_a_list():
    first = add_tag("urgent")
    second = add_tag("billing")
    assert first == ["urgent"]
    assert second == ["billing"]
    assert first is not second

def test_appends_to_the_given_list_in_place():
    tags = ["vip"]
    result = add_tag("refund", tags)
    assert tags == ["vip", "refund"]
    assert result is tags

def test_given_empty_list_is_used_not_replaced():
    tags: list[str] = []
    assert add_tag("new", tags) is tags
    assert tags == ["new"]

def test_explicit_none_means_a_new_list():
    assert add_tag("spam", None) == ["spam"]

def test_default_is_none():
    assert add_tag.__defaults__ == (None,)
