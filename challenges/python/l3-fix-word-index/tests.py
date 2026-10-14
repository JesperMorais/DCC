def test_no_lines_gives_an_empty_index():
    assert word_index([]) == {}

def test_blank_lines_add_nothing():
    assert word_index(["", "   "]) == {}

def test_a_single_line():
    assert word_index(["hello world"]) == {"hello": [1], "world": [1]}

def test_words_on_several_lines():
    assert word_index(["Reset the router", "Call the help desk"]) == {
        "reset": [1], "the": [1, 2], "router": [1],
        "call": [2], "help": [2], "desk": [2],
    }

def test_case_is_ignored():
    assert word_index(["Wifi down", "wifi UP", "WIFI"]) == {
        "wifi": [1, 2, 3], "down": [1], "up": [2],
    }

def test_a_repeated_word_lists_its_line_twice():
    assert word_index(["", "no no no"]) == {"no": [2, 2, 2]}
