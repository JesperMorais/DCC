function initials(first: string, last: string): string {
  const f = first.charAt(0).toUpperCase();
  const l = last.charAt(0).toUpperCase();
  return `${f}.${l}.`;
}
