function fullBoxes(items: number, perBox: number): number {
  return Math.floor(items / perBox);
}

function leftover(items: number, perBox: number): number {
  return items % perBox;
}

function boxesNeeded(items: number, perBox: number): number {
  return Math.ceil(items / perBox);
}
