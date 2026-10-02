type Size = [width: number, height: number];

function parseSize(text: string): Size | null {
  const parts = text.split("x");
  if (parts.length !== 2) return null;
  const [w, h] = parts.map((part) => (part.trim() === "" ? NaN : Number(part)));
  const valid = (n: number) => Number.isInteger(n) && n > 0;
  return valid(w) && valid(h) ? [w, h] : null;
}

function fitInside([w, h]: Size, [maxW, maxH]: Size): Size {
  const scale = Math.min(1, maxW / w, maxH / h);
  return [Math.round(w * scale), Math.round(h * scale)];
}
