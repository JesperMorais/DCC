def test_builds_one_dict_per_row():
    assert rows_to_dicts(["name", "role"], [["Ada", "admin"], ["Linus", "dev"]]) == [
        {"name": "Ada", "role": "admin"},
        {"name": "Linus", "role": "dev"},
    ]

def test_cleans_column_names():
    assert rows_to_dicts([" Name", "E-Mail  "], [["Ada", "ada@example.com"]]) == [
        {"name": "Ada", "e-mail": "ada@example.com"},
    ]

def test_strips_cell_values():
    assert rows_to_dicts(["sku", "qty"], [["  A-1 ", " 3"]]) == [{"sku": "A-1", "qty": "3"}]

def test_skips_rows_with_the_wrong_number_of_cells():
    rows = [["Ada", "admin"], ["Linus"], ["Grace", "dev", "extra"], [], ["Guido", "bdfl"]]
    assert rows_to_dicts(["name", "role"], rows) == [
        {"name": "Ada", "role": "admin"},
        {"name": "Guido", "role": "bdfl"},
    ]

def test_no_rows():
    assert rows_to_dicts(["name"], []) == []
