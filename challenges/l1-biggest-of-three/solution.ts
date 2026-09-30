function biggestOfThree(a: number, b: number, c: number): number {
  let biggest = a;
  if (b > biggest) {
    biggest = b;
  }
  if (c > biggest) {
    biggest = c;
  }
  return biggest;
}
