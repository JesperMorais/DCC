def test_greets_ada():
    assert greet("Ada") == "Hello, Ada!"


def test_greets_guido():
    assert greet("Guido") == "Hello, Guido!"


def test_handles_an_empty_name():
    assert greet("") == "Hello, stranger!"
