type Result<T, E> = { ok: true; value: T } | { ok: false; error: E };

function ok<T>(value: T): Result<T, never> {
  return { ok: true, value };
}

function err<E>(error: E): Result<never, E> {
  return { ok: false, error };
}

function parsePort(input: string): Result<number, string> {
  const s = input.trim();
  if (!/^\d+$/.test(s)) return err("not a number");
  const n = Number(s);
  if (n < 1 || n > 65535) return err("out of range");
  return ok(n);
}

function unwrapOr<T, E>(result: Result<T, E>, fallback: T): T {
  return result.ok ? result.value : fallback;
}
