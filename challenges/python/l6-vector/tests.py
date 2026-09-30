def test_addition_and_subtraction():
    assert Vector(1, 2) + Vector(3, 4) == Vector(4, 6)
    assert Vector(1, 2) - Vector(3, 4) == Vector(-2, -2)


def test_scaling_works_from_both_sides():
    assert Vector(1, 2) * 3 == Vector(3, 6)
    assert 3 * Vector(1, 2) == Vector(3, 6)
    assert Vector(4, -2) * 0.5 == Vector(2.0, -1.0)


def test_abs_is_the_length_and_zero_is_falsy():
    assert abs(Vector(3, 4)) == approx(5.0)
    assert abs(Vector(-1, 1)) == approx(1.41421, rel=1e-4)
    assert not Vector(0, 0)
    assert Vector(0, 0.1)


def test_repr_looks_like_the_constructor_call():
    assert repr(Vector(3, 4)) == "Vector(3, 4)"
    assert repr(Vector(0.5, -1.0)) == "Vector(0.5, -1.0)"


def test_equality_with_other_types_is_just_false():
    assert (Vector(1, 2) == (1, 2)) is False
    assert Vector(1, 2) != Vector(2, 1)


def test_unsupported_operands_raise_type_error():
    with raises(TypeError):
        Vector(1, 2) * Vector(3, 4)
    with raises(TypeError):
        Vector(1, 2) + 1
    with raises(TypeError):
        "a" * Vector(1, 2)


def test_immutable_and_hashable():
    v = Vector(1, 2)
    with raises(AttributeError):
        v.x = 5
    assert len({Vector(1, 2), Vector(1, 2), Vector(2, 1)}) == 2
