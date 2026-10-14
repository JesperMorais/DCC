function isWeekend(day: string): boolean {
  const name = day.trim();
  return name === "saturday" || name === "sunday";
}
