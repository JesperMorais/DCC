function hasItem(list: string[], item: string): boolean {
  return list.includes(item);
}

function positionOf(list: string[], item: string): number {
  return list.indexOf(item);
}

function addIfMissing(list: string[], item: string): string[] {
  if (list.includes(item)) {
    return list;
  }
  return [...list, item];
}
