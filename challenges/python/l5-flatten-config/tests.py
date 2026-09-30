def test_already_flat_config_is_unchanged():
    assert flatten_config({"debug": True, "port": 80}) == {"debug": True, "port": 80}


def test_one_level_of_nesting():
    config = {"db": {"host": "localhost", "port": 5432}, "debug": True}
    assert flatten_config(config) == {"db.host": "localhost", "db.port": 5432, "debug": True}


def test_arbitrary_depth():
    config = {"a": {"b": {"c": {"d": 1}}, "e": 2}}
    assert flatten_config(config) == {"a.b.c.d": 1, "a.e": 2}


def test_custom_separator():
    assert flatten_config({"a": {"b": {"c": 1}}}, sep="__") == {"a__b__c": 1}


def test_lists_and_none_are_leaves_and_empty_dicts_vanish():
    config = {"tags": ["x", {"y": 1}], "extra": {}, "owner": None}
    assert flatten_config(config) == {"tags": ["x", {"y": 1}], "owner": None}


def test_empty_config():
    assert flatten_config({}) == {}


def test_input_is_not_modified():
    config = {"db": {"host": "h"}}
    flatten_config(config)
    assert config == {"db": {"host": "h"}}
