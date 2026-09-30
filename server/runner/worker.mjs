// Runs transpiled user code + tests inside a fresh vm context.
// Lives in its own worker thread so an infinite loop can be killed from outside.
import { parentPort, workerData } from "node:worker_threads";
import vm from "node:vm";
import util from "node:util";

const { userJs, testsJs, syncTimeoutMs, testTimeoutMs } = workerData;

const logs = [];
const MAX_LOGS = 200;
const show = (v) => util.inspect(v, { depth: 4, breakLength: 80 });
const fmt = (v) => (typeof v === "string" ? v : util.inspect(v, { depth: 4, breakLength: 80 }));
const pushLog = (level) => (...args) => {
  if (logs.length < MAX_LOGS) logs.push({ level, text: args.map(fmt).join(" ") });
};
const sandboxConsole = {
  log: pushLog("log"),
  info: pushLog("log"),
  debug: pushLog("log"),
  warn: pushLog("warn"),
  error: pushLog("error"),
  table: pushLog("log"),
};

class AssertionError extends Error {
  constructor(message, expected, received) {
    super(message);
    this.name = "AssertionError";
    this.expected = expected;
    this.received = received;
  }
}

function deepEqual(a, b) {
  if (Object.is(a, b)) return true;
  if (typeof a !== typeof b || a === null || b === null || typeof a !== "object") return false;
  if (Array.isArray(a) !== Array.isArray(b)) return false;
  if (a instanceof Map && b instanceof Map) {
    if (a.size !== b.size) return false;
    for (const [k, v] of a) if (!b.has(k) || !deepEqual(v, b.get(k))) return false;
    return true;
  }
  if (a instanceof Set && b instanceof Set) {
    if (a.size !== b.size) return false;
    for (const v of a) if (!b.has(v)) return false;
    return true;
  }
  if (a instanceof Date && b instanceof Date) return a.getTime() === b.getTime();
  const ka = Object.keys(a).filter((k) => a[k] !== undefined);
  const kb = Object.keys(b).filter((k) => b[k] !== undefined);
  if (ka.length !== kb.length) return false;
  return ka.every((k) => Object.prototype.hasOwnProperty.call(b, k) && deepEqual(a[k], b[k]));
}

function makeMatchers(actual, negate) {
  const check = (pass, message, expected) => {
    if (pass === negate) {
      throw new AssertionError(negate ? `Expected NOT: ${message}` : message, expected === undefined ? undefined : show(expected), show(actual));
    }
  };
  const m = {
    toBe: (e) => check(Object.is(actual, e), `expected ${show(e)}, received ${show(actual)}`, e),
    toEqual: (e) => check(deepEqual(actual, e), `values are not deeply equal`, e),
    toBeCloseTo: (e, digits = 2) =>
      check(Math.abs(actual - e) < Math.pow(10, -digits) / 2, `expected ≈ ${e} (${digits} digits), received ${show(actual)}`, e),
    toBeTruthy: () => check(!!actual, `expected a truthy value, received ${show(actual)}`),
    toBeFalsy: () => check(!actual, `expected a falsy value, received ${show(actual)}`),
    toBeUndefined: () => check(actual === undefined, `expected undefined, received ${show(actual)}`),
    toBeNull: () => check(actual === null, `expected null, received ${show(actual)}`),
    toBeGreaterThan: (n) => check(actual > n, `expected > ${n}, received ${show(actual)}`),
    toBeLessThan: (n) => check(actual < n, `expected < ${n}, received ${show(actual)}`),
    toHaveLength: (n) => check(actual != null && actual.length === n, `expected length ${n}, received ${actual?.length}`, n),
    toContain: (item) =>
      check(
        typeof actual === "string" ? actual.includes(item) : Array.from(actual ?? []).some((x) => deepEqual(x, item)),
        `expected to contain ${show(item)}`,
        item,
      ),
    toBeInstanceOf: (ctor) => check(actual instanceof ctor, `expected instance of ${ctor?.name}`),
    toThrow: (msg) => {
      if (typeof actual !== "function") throw new AssertionError("toThrow() needs a function: expect(() => fn()).toThrow()");
      let threw = false;
      let err;
      try {
        actual();
      } catch (e) {
        threw = true;
        err = e;
      }
      let pass = threw;
      if (threw && msg !== undefined) {
        const text = String(err?.message ?? err);
        pass = msg instanceof RegExp ? msg.test(text) : text.includes(msg);
      }
      if (pass === negate) {
        throw new AssertionError(
          negate ? "expected function NOT to throw" : threw ? `threw "${err?.message ?? err}", which doesn't match ${show(msg)}` : "expected function to throw",
        );
      }
    },
  };
  return m;
}

const tests = [];
const context = vm.createContext({
  console: sandboxConsole,
  test: (name, fn) => tests.push({ name, fn }),
  expect: (actual) => {
    const m = makeMatchers(actual, false);
    m.not = makeMatchers(actual, true);
    return m;
  },
  setTimeout,
  clearTimeout,
  queueMicrotask,
  structuredClone,
});

function serializeError(e) {
  if (e && e.name === "AssertionError") return { message: e.message, expected: e.expected, received: e.received };
  const stack = String(e?.stack ?? "")
    .split("\n")
    .filter((l) => l.includes("your-code.js"))
    .slice(0, 3)
    .join("\n");
  return { message: `${e?.name ?? "Error"}: ${e?.message ?? String(e)}`, stack: stack || undefined };
}

async function main() {
  const setup = [];
  for (const [filename, code] of [
    ["your-code.js", userJs],
    ["tests.js", testsJs],
  ]) {
    try {
      vm.runInContext(code, context, { filename, timeout: syncTimeoutMs });
    } catch (e) {
      const timedOut = e?.code === "ERR_SCRIPT_EXECUTION_TIMEOUT";
      setup.push({
        file: filename.replace(".js", ""),
        ...serializeError(timedOut ? new Error(`Code took longer than ${syncTimeoutMs}ms — infinite loop?`) : e),
      });
      parentPort.postMessage({ setupErrors: setup, results: [], logs });
      return;
    }
  }

  const results = [];
  let looped = false;
  for (const t of tests) {
    // One infinite loop is enough to know; don't make the learner wait for every test to time out.
    if (looped) {
      results.push({ name: t.name, pass: false, ms: 0, error: { message: "Skipped: an earlier test looped forever." } });
      continue;
    }
    const started = performance.now();
    try {
      // Run sync part under the vm timeout so `while(true)` in a test is caught.
      context.__fn = t.fn;
      const ret = vm.runInContext("__fn()", context, { timeout: syncTimeoutMs });
      if (ret && typeof ret.then === "function") {
        await Promise.race([
          ret,
          new Promise((_, rej) => setTimeout(() => rej(new Error(`Test timed out after ${testTimeoutMs}ms`)), testTimeoutMs)),
        ]);
      }
      results.push({ name: t.name, pass: true, ms: performance.now() - started });
    } catch (e) {
      const timedOut = e?.code === "ERR_SCRIPT_EXECUTION_TIMEOUT";
      if (timedOut) looped = true;
      results.push({
        name: t.name,
        pass: false,
        ms: performance.now() - started,
        error: serializeError(timedOut ? new Error(`Took longer than ${syncTimeoutMs}ms — infinite loop?`) : e),
      });
    }
  }
  parentPort.postMessage({ setupErrors: setup, results, logs });
}

main();
