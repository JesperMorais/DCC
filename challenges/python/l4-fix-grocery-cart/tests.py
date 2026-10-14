def test_a_new_cart_is_empty():
    cart = GroceryCart("Ada")
    assert cart.item_count() == 0
    assert cart.total_cents() == 0


def test_adding_items_updates_count_and_total():
    cart = GroceryCart("Ada")
    cart.add("milk", 1490, 2)
    cart.add("bread", 3250)
    assert cart.item_count() == 3
    assert cart.total_cents() == 6230


def test_adding_the_same_product_again_merges_the_line():
    cart = GroceryCart("Ada")
    cart.add("eggs", 3900)
    cart.add("eggs", 3900, 2)
    assert len(cart.lines) == 1
    assert cart.lines[0].quantity == 3


def test_rejects_a_zero_quantity():
    cart = GroceryCart("Ada")
    with raises(ValueError, match="at least 1"):
        cart.add("milk", 1490, 0)


def test_two_customers_have_separate_carts():
    ada = GroceryCart("Ada")
    linus = GroceryCart("Linus")
    ada.add("coffee", 5900)
    linus.add("tea", 3400, 2)
    assert (ada.item_count(), ada.total_cents()) == (1, 5900)
    assert (linus.item_count(), linus.total_cents()) == (2, 6800)
