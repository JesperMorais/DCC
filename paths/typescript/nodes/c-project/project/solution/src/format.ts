/** 123456 → "1234.56", -8950 → "-89.50" */
export const money = (cents: number): string => (cents / 100).toFixed(2);

/** One line of the report: the name in a 14-wide column, the amount right-aligned in 10. */
export const reportLine = (name: string, cents: number): string => name.padEnd(14) + money(cents).padStart(10);
