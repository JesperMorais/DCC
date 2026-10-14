def test_the_hash_sign_and_spaces_are_ignored():
    assert Hashtag("#fika") == Hashtag(" fika ")


def test_comparison_ignores_case():
    assert Hashtag("#Python") == Hashtag("#python")
    assert Hashtag("#Python") != Hashtag("#rust")


def test_unique_hashtags_drops_exact_duplicates():
    assert unique_hashtags(["#fika", "#fika", "#kanelbulle"]) == {Hashtag("fika"), Hashtag("kanelbulle")}


def test_unique_hashtags_ignores_case():
    assert len(unique_hashtags(["#Python", "#python", "#PYTHON", "#rust"])) == 2


def test_hashtags_work_as_dict_keys():
    followers = {Hashtag("#Stockholm"): 120}
    assert followers.get(Hashtag("#stockholm")) == 120


def test_trending_counts_every_spelling_together():
    posts = [["#Python", "#async"], ["#python"], ["#PYTHON", "#Async"], ["#rust"]]
    assert trending(posts, top=2) == [("Python", 3), ("async", 2)]


def test_empty_hashtag_is_rejected():
    with raises(ValueError, match="empty"):
        Hashtag("#")
