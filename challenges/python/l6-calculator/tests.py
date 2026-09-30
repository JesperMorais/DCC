def test_precedence_and_left_to_right():
    assert calculate("2 + 3 * 4") == 14
    assert calculate("10 - 2 - 3") == 5
    assert calculate("8 / 4 / 2") == 1
    assert calculate("2*3+4*5") == 26


def test_parentheses_group_and_nest():
    assert calculate("(2 + 3) * 4") == 20
    assert calculate("2 * (3 + (4 - 1)) / 3") == 4
    assert calculate("((7))") == 7
    assert calculate("(" * 40 + "1" + ")" * 40) == 1


def test_decimals_and_true_division():
    assert calculate(" 1.5*2 ") == approx(3.0)
    assert calculate("7 / 2") == approx(3.5)
    assert isinstance(calculate("1+1"), float)


def test_malformed_expressions_raise_value_error():
    for bad in ["", "3 +", "* 2", "2 ** 3", "3 4", "(1 + 2", "1 + 2)", "()", "(3)(4)", "3 + x"]:
        with raises(ValueError):
            calculate(bad)


def test_division_by_zero():
    with raises(ZeroDivisionError):
        calculate("1 / (2 - 2)")
