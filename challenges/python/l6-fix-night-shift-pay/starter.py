from dataclasses import dataclass
from datetime import datetime, timedelta
from zoneinfo import ZoneInfo

STOCKHOLM = ZoneInfo("Europe/Stockholm")
OVERTIME_FACTOR = 1.5


@dataclass(frozen=True)
class Shift:
    employee: str
    start: datetime
    end: datetime

    def __post_init__(self) -> None:
        if self.start.tzinfo is None or self.end.tzinfo is None:
            raise ValueError("shift times must be timezone-aware")
        if self.end <= self.start:
            raise ValueError("a shift must end after it starts")

    @property
    def hours(self) -> float:
        return (self.end - self.start) / timedelta(hours=1)


def parse_shift(employee: str, start: str, end: str, tz: ZoneInfo = STOCKHOLM) -> Shift:
    return Shift(
        employee,
        datetime.fromisoformat(start).replace(tzinfo=tz),
        datetime.fromisoformat(end).replace(tzinfo=tz),
    )


@dataclass(frozen=True)
class Payslip:
    employee: str
    regular_hours: float
    overtime_hours: float
    gross_cents: int


def weekly_payslip(shifts: list[Shift], hourly_cents: int, overtime_after: float = 40.0) -> Payslip:
    if not shifts:
        raise ValueError("no shifts to pay")
    employees = {s.employee for s in shifts}
    if len(employees) != 1:
        raise ValueError(f"shifts for several employees: {sorted(employees)}")
    total = sum(s.hours for s in shifts)
    regular = min(total, overtime_after)
    overtime = total - regular
    gross = round(regular * hourly_cents + overtime * hourly_cents * OVERTIME_FACTOR)
    return Payslip(employees.pop(), regular, overtime, gross)
