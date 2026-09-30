type Token =
  | { kind: "num"; value: number }
  | { kind: "op"; op: "+" | "-" | "*" | "/" };

function tokenize(expr: string): Token[] {
  // TODO
  return [];
}

function evaluate(expr: string): number {
  // TODO
  return 0;
}
