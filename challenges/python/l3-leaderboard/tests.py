def test_higher_score_first():
    assert leaderboard([("ada", 50, 10.0), ("linus", 90, 99.0), ("grace", 70, 5.0)]) == [
        "1. linus (90 pts)",
        "2. grace (70 pts)",
        "3. ada (50 pts)",
    ]

def test_same_score_faster_player_first():
    assert leaderboard([("ada", 80, 42.0), ("linus", 95, 60.5), ("grace", 80, 39.9)]) == [
        "1. linus (95 pts)",
        "2. grace (80 pts)",
        "3. ada (80 pts)",
    ]

def test_full_tie_is_broken_by_name():
    assert leaderboard([("zoe", 60, 30.0), ("amy", 60, 30.0), ("max", 60, 30.0)]) == [
        "1. amy (60 pts)",
        "2. max (60 pts)",
        "3. zoe (60 pts)",
    ]

def test_does_not_reorder_the_input():
    results = [("b", 1, 1.0), ("a", 2, 1.0)]
    leaderboard(results)
    assert results == [("b", 1, 1.0), ("a", 2, 1.0)]

def test_single_and_empty():
    assert leaderboard([("solo", 0, 12.5)]) == ["1. solo (0 pts)"]
    assert leaderboard([]) == []
