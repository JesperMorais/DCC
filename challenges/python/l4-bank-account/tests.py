def test_starts_empty():
    assert Account("Ada").balance == 0


def test_deposit_and_withdraw_return_the_new_balance():
    acct = Account("Ada")
    assert acct.deposit(500) == 500
    assert acct.withdraw(200) == 300
    assert acct.balance == 300


def test_rejects_non_positive_amounts():
    acct = Account("Ada")
    with raises(ValueError, match="positive"):
        acct.deposit(0)
    with raises(ValueError, match="positive"):
        acct.withdraw(-5)


def test_overdraft_is_refused_and_balance_is_unchanged():
    acct = Account("Ada")
    acct.deposit(100)
    with raises(ValueError, match="insufficient funds"):
        acct.withdraw(101)
    assert acct.balance == 100


def test_balance_is_read_only():
    acct = Account("Ada")
    with raises(AttributeError):
        acct.balance = 1_000_000
    assert acct.balance == 0


def test_accounts_are_independent():
    a, b = Account("Ada"), Account("Linus")
    a.deposit(50)
    assert (a.balance, b.balance) == (50, 0)
