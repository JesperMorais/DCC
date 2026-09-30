def test_strips_and_lowercases():
    assert clean_tags(["  Python", "BEGINNER "]) == ["python", "beginner"]


def test_drops_empty_tags():
    assert clean_tags(["  Python", "BEGINNER ", ""]) == ["python", "beginner"]


def test_drops_tags_that_are_only_spaces():
    assert clean_tags(["   "]) == []


def test_keeps_order_and_duplicates():
    assert clean_tags(["Go", "rust", "go"]) == ["go", "rust", "go"]


def test_empty_list():
    assert clean_tags([]) == []


def test_does_not_change_the_original_list():
    original = [" A ", "b"]
    clean_tags(original)
    assert original == [" A ", "b"]
