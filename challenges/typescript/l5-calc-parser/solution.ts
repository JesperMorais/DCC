type Token =
  | { kind: "num"; value: number }
  | { kind: "op"; op: "+" | "-" | "*" | "/" };

function tokenize(expr: string): Token[] {
  const tokens: Token[] = [];
  let i = 0;
  while (i < expr.length) {
    const c = expr[i];
    if (/\s/.test(c)) {
      i++;
    } else if (c >= "0" && c <= "9") {
      let j = i;
      while (j < expr.length && expr[j] >= "0" && expr[j] <= "9") j++;
      tokens.push({ kind: "num", value: Number(expr.slice(i, j)) });
      i = j;
    } else if (c === "+" || c === "-" || c === "*" || c === "/") {
      tokens.push({ kind: "op", op: c });
      i++;
    } else {
      throw new Error(`Unexpected character "${c}"`);
    }
  }
  return tokens;
}

function evaluate(expr: string): number {
  const tokens = tokenize(expr);
  const numAt = (i: number): number => {
    const t = tokens[i];
    if (!t || t.kind !== "num") throw new Error("Expected a number");
    return t.value;
  };

  const terms = [numAt(0)];
  const addOps: ("+" | "-")[] = [];
  for (let i = 1; i < tokens.length; i += 2) {
    const t = tokens[i];
    if (t.kind !== "op") throw new Error("Expected an operator");
    const n = numAt(i + 1);
    if (t.op === "*") terms[terms.length - 1] *= n;
    else if (t.op === "/") terms[terms.length - 1] /= n;
    else {
      addOps.push(t.op);
      terms.push(n);
    }
  }

  let result = terms[0];
  addOps.forEach((op, k) => {
    result = op === "+" ? result + terms[k + 1] : result - terms[k + 1];
  });
  return result;
}
