from datetime import datetime


def test_a_day_shift():
    assert parse_shift("Sam", "2026-10-14 08:00", "2026-10-14 16:30").hours == 8.5


def test_an_ordinary_night_shift():
    assert parse_shift("Sam", "2026-10-17 22:00", "2026-10-18 06:00").hours == 8.0


def test_night_shift_when_the_clocks_go_back():
    # 25 Oct 2026: at 03:00 Swedish clocks are set back to 02:00
    assert parse_shift("Sam", "2026-10-24 22:00", "2026-10-25 06:00").hours == 9.0


def test_night_shift_when_the_clocks_go_forward():
    # 29 Mar 2026: at 02:00 Swedish clocks jump to 03:00
    assert parse_shift("Sam", "2026-03-28 22:00", "2026-03-29 06:00").hours == 7.0


def test_overtime_after_forty_hours():
    week = [parse_shift("Sam", f"2026-10-{d} 08:00", f"2026-10-{d} 17:00") for d in range(12, 17)]
    assert weekly_payslip(week, hourly_cents=18_000) == Payslip("Sam", 40.0, 5.0, 720_000 + 135_000)


def test_a_week_with_the_long_night_is_paid_in_full():
    week = [parse_shift("Kim", f"2026-10-{d} 22:00", f"2026-10-{d + 1} 06:00") for d in range(20, 25)]
    slip = weekly_payslip(week, hourly_cents=20_000)
    assert (slip.regular_hours, slip.overtime_hours) == (40.0, 1.0)
    assert slip.gross_cents == 800_000 + 30_000


def test_invalid_shifts_and_weeks_are_rejected():
    with raises(ValueError, match="timezone-aware"):
        Shift("Sam", datetime(2026, 10, 14, 8), datetime(2026, 10, 14, 16))
    with raises(ValueError, match="end after"):
        parse_shift("Sam", "2026-10-14 16:00", "2026-10-14 08:00")
    with raises(ValueError, match="several employees"):
        weekly_payslip([parse_shift("Sam", "2026-10-14 08:00", "2026-10-14 16:00"),
                        parse_shift("Kim", "2026-10-14 08:00", "2026-10-14 16:00")], hourly_cents=18_000)
    with raises(ValueError, match="no shifts"):
        weekly_payslip([], hourly_cents=18_000)
