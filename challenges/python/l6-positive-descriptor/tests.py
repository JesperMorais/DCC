class Product:
    price = Positive()
    weight = Positive()

    def __init__(self, name: str, price: float, weight: float) -> None:
        self.name = name
        self.price = price
        self.weight = weight


def test_valid_values_are_stored_and_read_back():
    p = Product("kettle", 249, 1.2)
    assert (p.price, p.weight) == (249, 1.2)
    p.price = 199.5
    assert p.price == 199.5


def test_rejects_zero_and_negatives_even_in_init():
    with raises(ValueError, match="price must be positive"):
        Product("bad", -1, 1.0)
    p = Product("kettle", 249, 1.2)
    with raises(ValueError, match="weight must be positive"):
        p.weight = 0


def test_a_rejected_value_leaves_the_old_one():
    p = Product("kettle", 249, 1.2)
    with raises(ValueError):
        p.price = -5
    assert p.price == 249


def test_rejects_non_numbers_including_bools():
    p = Product("kettle", 249, 1.2)
    with raises(TypeError, match="price must be a number"):
        p.price = "12"
    with raises(TypeError, match="weight must be a number"):
        p.weight = True


def test_each_instance_and_attribute_has_its_own_value():
    a = Product("a", 10, 1)
    b = Product("b", 20, 2)
    assert (a.price, a.weight, b.price, b.weight) == (10, 1, 20, 2)


def test_class_access_returns_the_descriptor():
    assert isinstance(Product.price, Positive)
    assert Product.price is not Product.weight


def test_reading_before_setting_is_an_attribute_error():
    class Box:
        width = Positive()

    with raises(AttributeError):
        Box().width
