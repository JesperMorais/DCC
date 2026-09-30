RESULTS = ["a", "b", "c", "d", "e"]

def test_first_page():
    assert paginate(RESULTS, 1, 2) == ["a", "b"]

def test_middle_page():
    assert paginate(RESULTS, 2, 2) == ["c", "d"]

def test_last_page_can_be_short():
    assert paginate(RESULTS, 3, 2) == ["e"]
    assert paginate(RESULTS, 1, 10) == RESULTS

def test_page_past_the_end_is_empty():
    assert paginate(RESULTS, 4, 2) == []
    assert paginate([], 1, 20) == []

def test_rejects_page_or_size_below_one():
    with raises(ValueError, match="at least 1"):
        paginate(RESULTS, 0, 2)
    with raises(ValueError, match="at least 1"):
        paginate(RESULTS, 1, 0)
    with raises(ValueError, match="at least 1"):
        paginate(RESULTS, -1, 5)

def test_does_not_change_the_input():
    items = ["x", "y", "z"]
    page = paginate(items, 1, 2)
    page.append("new")
    assert items == ["x", "y", "z"]
