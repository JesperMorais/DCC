def team(name, won, drawn, lost, goals_for, goals_against):
    return TeamRecord(name, won, drawn, lost, goals_for, goals_against)


def test_more_points_ranks_higher():
    table = league_table([
        team("Hammarby", 1, 1, 2, 5, 6),
        team("Malmö", 3, 0, 1, 9, 3),
        team("AIK", 2, 1, 1, 6, 4),
    ])
    assert table == ["1. Malmö 9", "2. AIK 7", "3. Hammarby 4"]


def test_equal_points_are_split_by_goal_difference():
    table = league_table([
        team("Elfsborg", 2, 0, 2, 6, 6),
        team("Häcken", 2, 0, 2, 8, 5),
    ])
    assert table == ["1. Häcken 6", "2. Elfsborg 6"]


def test_full_ties_are_listed_alphabetically():
    table = league_table([
        team("Sirius", 1, 2, 1, 4, 4),
        team("Djurgården", 1, 2, 1, 3, 3),
        team("Mjällby", 1, 2, 1, 5, 5),
    ])
    assert table == ["1. Djurgården 5", "2. Mjällby 5", "3. Sirius 5"]


def test_a_tie_at_the_top_with_a_leader_below():
    table = league_table([
        team("Örebro", 0, 0, 4, 1, 9),
        team("Brommapojkarna", 2, 2, 0, 5, 2),
        team("Brage", 2, 2, 0, 6, 3),
    ])
    assert table == ["1. Brage 8", "2. Brommapojkarna 8", "3. Örebro 0"]


def test_empty_league():
    assert league_table([]) == []
