function isWeekend(day: string): boolean {
  const name = day.trim().toLowerCase();
  return name === "saturday" || name === "sunday";
}
