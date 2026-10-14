def test_cards_go_to_the_first_column_by_default():
    board = KanbanBoard(["Todo", "Doing", "Done"])
    board.add_card("write tests")
    board.add_card("deploy", "Doing")
    assert board.snapshot() == {"Todo": ["write tests"], "Doing": ["deploy"], "Done": []}


def test_moving_a_card_between_columns():
    board = KanbanBoard(["Todo", "Done"])
    board.add_card("a")
    board.add_card("b")
    board.move_card("b", "Done")
    board.move_card("a", "Done", position=0)
    assert board.snapshot() == {"Todo": [], "Done": ["a", "b"]}


def test_undo_removes_an_added_column():
    board = KanbanBoard(["Todo"])
    board.add_column("Review")
    assert board.undo() is True
    assert board.snapshot() == {"Todo": []}
    assert board.undo() is False


def test_undo_reverts_a_move():
    board = KanbanBoard(["Todo", "Doing"])
    board.add_card("fix login")
    board.move_card("fix login", "Doing")
    assert board.undo() is True
    assert board.snapshot() == {"Todo": ["fix login"], "Doing": []}


def test_undo_steps_back_one_change_at_a_time():
    board = KanbanBoard(["Todo", "Done"])
    board.add_card("a")
    board.add_card("b")
    board.move_card("a", "Done")
    board.undo()
    assert board.snapshot() == {"Todo": ["a", "b"], "Done": []}
    board.undo()
    assert board.snapshot() == {"Todo": ["a"], "Done": []}
    board.undo()
    assert board.snapshot() == {"Todo": [], "Done": []}


def test_history_is_capped():
    board = KanbanBoard(["Todo"], max_undo=2)
    for name in ["A", "B", "C"]:
        board.add_column(name)
    assert board.undo() and board.undo()
    assert board.undo() is False
    assert board.snapshot() == {"Todo": [], "A": []}


def test_invalid_changes_are_rejected():
    board = KanbanBoard(["Todo"])
    board.add_card("a")
    with raises(ValueError, match="duplicate"):
        board.add_card("a")
    with raises(KeyError, match="Nope"):
        board.move_card("a", "Nope")
    with raises(KeyError, match="ghost"):
        board.move_card("ghost", "Todo")
    assert board.snapshot() == {"Todo": ["a"]}
