// Type declarations for the tiny test API available to every challenge.
// Kept as a string so the type-checker (server) and Monaco (browser) can both load it.
export const HARNESS_DTS = `
declare const console: {
  log(...data: unknown[]): void;
  info(...data: unknown[]): void;
  debug(...data: unknown[]): void;
  warn(...data: unknown[]): void;
  error(...data: unknown[]): void;
  table(...data: unknown[]): void;
};
declare function setTimeout(handler: (...args: any[]) => void, ms?: number, ...args: any[]): number;
declare function clearTimeout(id: number | undefined): void;
declare function queueMicrotask(callback: () => void): void;
declare function structuredClone<T>(value: T): T;
interface Matchers<T> {
  /** Strict equality (Object.is). Use for numbers, strings, booleans. */
  toBe(expected: T): void;
  /** Deep equality. Use for arrays and objects. */
  toEqual(expected: T): void;
  toBeCloseTo(expected: number, digits?: number): void;
  toBeTruthy(): void;
  toBeFalsy(): void;
  toBeUndefined(): void;
  toBeNull(): void;
  toBeGreaterThan(n: number): void;
  toBeLessThan(n: number): void;
  toHaveLength(n: number): void;
  toContain(item: unknown): void;
  toBeInstanceOf(ctor: abstract new (...args: any[]) => unknown): void;
  /** Pass a function: expect(() => fn()).toThrow() */
  toThrow(message?: string | RegExp): void;
  not: Omit<Matchers<T>, "not">;
}
declare function test(name: string, fn: () => void | Promise<void>): void;
declare function expect<T>(actual: T): Matchers<T>;

/** Type-level test helpers */
type Equal<X, Y> = (<T>() => T extends X ? 1 : 2) extends (<T>() => T extends Y ? 1 : 2) ? true : false;
type NotEqual<X, Y> = true extends Equal<X, Y> ? false : true;
type Expect<T extends true> = T;
type ExpectFalse<T extends false> = T;
type IsAny<T> = 0 extends 1 & T ? true : false;
`;
