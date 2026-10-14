def test_removes_a_single_finished_task():
    tasks = ["[ ] buy milk", "[x] call mum", "[ ] fix bike"]
    clear_finished(tasks)
    assert tasks == ["[ ] buy milk", "[ ] fix bike"]

def test_open_tasks_are_left_alone():
    tasks = ["[ ] a", "[ ] b"]
    clear_finished(tasks)
    assert tasks == ["[ ] a", "[ ] b"]

def test_an_empty_list_stays_empty():
    tasks: list[str] = []
    clear_finished(tasks)
    assert tasks == []

def test_two_finished_tasks_in_a_row():
    tasks = ["[x] pay rent", "[x] water plants", "[ ] book dentist"]
    clear_finished(tasks)
    assert tasks == ["[ ] book dentist"]

def test_everything_finished():
    tasks = ["[x] a", "[x] b", "[x] c", "[x] d"]
    clear_finished(tasks)
    assert tasks == []

def test_returns_nothing():
    assert clear_finished(["[x] a"]) is None
