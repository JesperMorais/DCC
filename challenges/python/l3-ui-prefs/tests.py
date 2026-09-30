def test_reads_both_values():
    assert ui_prefs({"settings": {"ui": {"theme": "dark", "font_size": 16}}}) == ("dark", 16)

def test_each_value_defaults_on_its_own():
    assert ui_prefs({"settings": {"ui": {"theme": "dark"}}}) == ("dark", 14)
    assert ui_prefs({"settings": {"ui": {"font_size": 20}}}) == ("light", 20)

def test_missing_sections_use_defaults():
    assert ui_prefs({"settings": {"language": "sv"}}) == ("light", 14)
    assert ui_prefs({"name": "Ada"}) == ("light", 14)

def test_null_sections_use_defaults():
    assert ui_prefs({"settings": None}) == ("light", 14)
    assert ui_prefs({"settings": {"ui": None}}) == ("light", 14)

def test_empty_profile():
    assert ui_prefs({}) == ("light", 14)

def test_does_not_add_keys_to_the_profile():
    profile = {"settings": {}}
    ui_prefs(profile)
    assert profile == {"settings": {}}
