def bill_with_tip(bill: float, tip_percent: float) -> float:
    tip = bill * tip_percent / 100
    return bill + tip
