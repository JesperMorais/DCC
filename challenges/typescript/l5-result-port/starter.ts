type Result<T, E> = any; // TODO: make this a proper union

function ok<T>(value: T): Result<T, never> {
  return undefined;
}

function err<E>(error: E): Result<never, E> {
  return undefined;
}

function parsePort(input: string): Result<number, string> {
  return undefined;
}

function unwrapOr<T, E>(result: Result<T, E>, fallback: T): T {
  return fallback;
}
