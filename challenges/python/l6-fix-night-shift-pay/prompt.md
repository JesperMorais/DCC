**Bug report from the night-shift staff at a Stockholm warehouse:** "Last weekend was the long night: the clocks went back an hour, and we worked 22:00 to 06:00. That's nine hours on the job. The payslip says eight. And in March, the people on the short night got paid an hour they never worked."

`parse_shift(employee, start, end)` reads local Stockholm wall-clock times (`"YYYY-MM-DD HH:MM"`) into a `Shift`. `Shift.hours` is how long the shift **actually lasted**: real elapsed time, as a float. A shift needs timezone-aware times and must end after it starts (`ValueError` otherwise).

`weekly_payslip(shifts, hourly_cents, overtime_after=40.0)` pays one employee's week: the first 40 hours at the hourly rate, every hour after that at 1.5×. It rejects an empty list or shifts for several employees.

```python
parse_shift("Sam", "2026-10-14 08:00", "2026-10-14 16:30").hours   # 8.5
parse_shift("Sam", "2026-10-17 22:00", "2026-10-18 06:00").hours   # 8.0

week = [parse_shift("Sam", f"2026-10-{d} 08:00", f"2026-10-{d} 17:00") for d in range(12, 17)]
weekly_payslip(week, hourly_cents=18_000)
# Payslip(employee="Sam", regular_hours=40.0, overtime_hours=5.0, gross_cents=855000)
```

**Reproduction:** on 25 October 2026, Swedish clocks go from 03:00 back to 02:00.

```python
parse_shift("Sam", "2026-10-24 22:00", "2026-10-25 06:00").hours
# returns 8.0, expected 9.0
```
