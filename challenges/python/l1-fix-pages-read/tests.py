def test_three_days():
    assert total_pages_read(3) == 6

def test_a_single_day():
    assert total_pages_read(1) == 1

def test_zero_days_means_zero_pages():
    assert total_pages_read(0) == 0

def test_a_full_week():
    assert total_pages_read(7) == 28

def test_a_whole_month():
    assert total_pages_read(30) == 465
