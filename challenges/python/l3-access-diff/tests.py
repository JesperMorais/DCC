def test_reports_added_removed_and_kept():
    assert access_diff(["ada", "linus", "grace"], ["grace", "guido", "ada"]) == {
        "added": ["guido"],
        "removed": ["linus"],
        "kept": ["ada", "grace"],
    }

def test_lists_are_sorted():
    result = access_diff(["zoe", "mia"], ["zoe", "mia", "tom", "ann"])
    assert result["added"] == ["ann", "tom"]
    assert result["kept"] == ["mia", "zoe"]

def test_duplicates_are_reported_once():
    assert access_diff(["ada", "ada", "bob"], ["bob", "cy", "cy", "bob"]) == {
        "added": ["cy"],
        "removed": ["ada"],
        "kept": ["bob"],
    }

def test_no_change():
    assert access_diff(["ada", "bob"], ["bob", "ada"]) == {
        "added": [],
        "removed": [],
        "kept": ["ada", "bob"],
    }

def test_empty_inputs_still_have_all_keys():
    assert access_diff([], []) == {"added": [], "removed": [], "kept": []}
    assert access_diff([], ["ada"]) == {"added": ["ada"], "removed": [], "kept": []}
