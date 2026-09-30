import enum

def test_is_an_enum_with_the_right_values():
    assert issubclass(OrderStatus, enum.Enum)
    assert [s.value for s in OrderStatus] == ["pending", "paid", "shipped", "delivered"]
    assert [s.name for s in OrderStatus] == ["PENDING", "PAID", "SHIPPED", "DELIVERED"]

def test_advance_walks_the_workflow():
    assert OrderStatus.PENDING.advance() is OrderStatus.PAID
    assert OrderStatus.PAID.advance() is OrderStatus.SHIPPED
    assert OrderStatus.SHIPPED.advance() is OrderStatus.DELIVERED

def test_delivered_is_final():
    with raises(ValueError, match="final"):
        OrderStatus.DELIVERED.advance()

def test_parse_status_ignores_case_and_whitespace():
    assert parse_status("paid") is OrderStatus.PAID
    assert parse_status("  Shipped ") is OrderStatus.SHIPPED
    assert parse_status("DELIVERED") is OrderStatus.DELIVERED

def test_parse_status_rejects_unknown_text():
    with raises(ValueError):
        parse_status("shiped")
    with raises(ValueError):
        parse_status("")
