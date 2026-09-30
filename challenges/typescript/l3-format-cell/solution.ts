type CellValue = string | number | boolean | null;

function formatCell(value: CellValue): string {
  if (value === null) return "N/A";
  if (typeof value === "number") return value.toFixed(2);
  if (typeof value === "boolean") return value ? "Yes" : "No";
  const trimmed = value.trim();
  return trimmed === "" ? "N/A" : trimmed;
}

function formatRow(row: CellValue[]): string {
  return row.map(formatCell).join(" | ");
}
