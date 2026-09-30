"""daily.ts Python test harness.

Runs the learner's code, then pytest-style tests (plain `assert`, `raises`, `approx`)
in the same namespace. Asserts are rewritten via the AST so a failure reports
expected vs. received values, like pytest does. Results go to fd 3 as JSON.

usage: python3 -I harness.py your-code.py tests.py
"""

import ast
import asyncio
import builtins
import inspect
import io
import json
import math
import os
import re
import resource
import signal
import sys
import time
import traceback
import types

SYNC_TIMEOUT = 1.5
LOOP_MSG = "Took longer than 1500ms — infinite loop?"
SKIPPED_MSG = "Skipped: an earlier test looped forever."
MAX_LOGS = 200


class DtsTimeout(BaseException):
    """BaseException so a learner's `except Exception` can't swallow it."""


class DtsAssertion(AssertionError):
    def __init__(self, message, expected=None, received=None):
        super().__init__(message)
        self.message = message
        self.expected = expected
        self.received = received


def _alarm(_sig, _frame):
    raise DtsTimeout()


def show(v):
    r = repr(v)
    return r if len(r) <= 2000 else r[:2000] + "…"


# ---------------------------------------------------------------- test helpers
class approx:
    """`assert area(2) == approx(12.566, rel=1e-3)`"""

    def __init__(self, expected, rel=1e-6, abs=1e-12):
        self.expected, self.rel, self.abs = expected, rel, abs

    def _close(self, a, e):
        return math.isclose(a, e, rel_tol=self.rel, abs_tol=self.abs)

    def __eq__(self, actual):
        e = self.expected
        if isinstance(e, (list, tuple)):
            return isinstance(actual, (list, tuple)) and len(actual) == len(e) and all(self._close(a, x) for a, x in zip(actual, e))
        try:
            return self._close(actual, e)
        except TypeError:
            return False

    def __repr__(self):
        return f"approx({self.expected!r})"


class raises:
    """`with raises(ValueError, match="negative"): sqrt(-1)`"""

    def __init__(self, exc_type, match=None):
        self.exc_type, self.match = exc_type, match

    def __enter__(self):
        return self

    def __exit__(self, et, ev, tb):
        name = getattr(self.exc_type, "__name__", str(self.exc_type))
        if et is None:
            raise DtsAssertion(f"expected {name} to be raised, but nothing was raised")
        if et is DtsTimeout or not issubclass(et, self.exc_type):
            return False  # propagate the unexpected exception
        if self.match is not None and not re.search(self.match, str(ev)):
            raise DtsAssertion(f"{name} was raised, but its message {str(ev)!r} doesn't match {self.match!r}")
        self.value = ev
        return True


_OPS = {
    ast.Eq: ("==", lambda a, b: a == b),
    ast.NotEq: ("!=", lambda a, b: a != b),
    ast.Lt: ("<", lambda a, b: a < b),
    ast.LtE: ("<=", lambda a, b: a <= b),
    ast.Gt: (">", lambda a, b: a > b),
    ast.GtE: (">=", lambda a, b: a >= b),
    ast.In: ("in", lambda a, b: a in b),
    ast.NotIn: ("not in", lambda a, b: a not in b),
    ast.Is: ("is", lambda a, b: a is b),
    ast.IsNot: ("is not", lambda a, b: a is not b),
}
_OP_BY_NAME = {name: fn for name, fn in _OPS.values()}


def _dts_cmp(left, op, right, src, msg=None):
    if _OP_BY_NAME[op](left, right):
        return
    text = f"assert {src}" + (f" — {msg}" if msg else "")
    if op == "==":
        raise DtsAssertion(text, expected=show(right), received=show(left))
    raise DtsAssertion(f"{text}\n  left:  {show(left)}\n  right: {show(right)}")


def _dts_true(value, src, msg=None):
    if not value:
        raise DtsAssertion(f"assert {src}" + (f" — {msg}" if msg else "") + f"\n  was: {show(value)}")


class AssertRewriter(ast.NodeTransformer):
    def __init__(self, source):
        self.source = source

    def visit_Assert(self, node):
        src = ast.get_source_segment(self.source, node.test) or "…"
        msg = node.msg or ast.Constant(None)
        t = node.test
        if isinstance(t, ast.Compare) and len(t.ops) == 1 and type(t.ops[0]) in _OPS:
            call = ast.Call(
                func=ast.Name("_dts_cmp", ast.Load()),
                args=[t.left, ast.Constant(_OPS[type(t.ops[0])][0]), t.comparators[0], ast.Constant(src), msg],
                keywords=[],
            )
        else:
            call = ast.Call(func=ast.Name("_dts_true", ast.Load()), args=[t, ast.Constant(src), msg], keywords=[])
        return ast.copy_location(ast.Expr(call), node)


# ---------------------------------------------------------------- output capture
class Capture(io.TextIOBase):
    def __init__(self, logs, level):
        self.logs, self.level, self.buf = logs, level, ""

    def writable(self):
        return True

    def write(self, s):
        self.buf += s
        while "\n" in self.buf:
            line, self.buf = self.buf.split("\n", 1)
            if len(self.logs) < MAX_LOGS:
                self.logs.append({"level": self.level, "text": line})
        return len(s)

    def flush_rest(self):
        if self.buf:
            self.logs.append({"level": self.level, "text": self.buf})
            self.buf = ""


def user_frames(tb):
    frames = [f for f in traceback.extract_tb(tb) if f.filename == "your-code.py"]
    return "\n".join(f"your-code.py line {f.lineno}, in {f.name}: {(f.line or '').strip()}" for f in frames[-3:]) or None


def describe(e):
    if isinstance(e, DtsTimeout):
        return {"message": f"Error: {LOOP_MSG}"}
    if isinstance(e, DtsAssertion):
        out = {"message": e.message}
        if e.expected is not None:
            out["expected"], out["received"] = e.expected, e.received
        return out
    if isinstance(e, RecursionError):
        return {"message": "RecursionError: maximum recursion depth exceeded — is there a base case?", "stack": user_frames(e.__traceback__)}
    return {"message": f"{type(e).__name__}: {e}", "stack": user_frames(e.__traceback__)}


def run_limited(fn):
    signal.setitimer(signal.ITIMER_REAL, SYNC_TIMEOUT)
    try:
        result = fn()
        if inspect.iscoroutine(result):
            asyncio.run(result)
    finally:
        signal.setitimer(signal.ITIMER_REAL, 0)


def main():
    user_path, tests_path = sys.argv[1], sys.argv[2]
    out = os.fdopen(3, "w")
    try:
        resource.setrlimit(resource.RLIMIT_AS, (1 << 30, 1 << 30))
        resource.setrlimit(resource.RLIMIT_CPU, (10, 10))
    except (ValueError, OSError):
        pass
    signal.signal(signal.SIGALRM, _alarm)
    sys.setrecursionlimit(2000)

    logs = []
    report = {"diagnostics": [], "setupErrors": [], "results": [], "logs": logs}
    stdout, stderr = Capture(logs, "log"), Capture(logs, "error")
    real_stdout = sys.stdout

    def finish():
        stdout.flush_rest()
        stderr.flush_rest()
        out.write(json.dumps(report))
        out.flush()

    user_src = open(user_path, encoding="utf8").read()
    tests_src = open(tests_path, encoding="utf8").read()

    try:
        user_code = compile(user_src, "your-code.py", "exec")
    except SyntaxError as e:
        report["diagnostics"].append(
            {
                "file": "your-code",
                "line": e.lineno or 1,
                "column": e.offset or 1,
                "message": f"{type(e).__name__}: {e.msg}",
                "code": type(e).__name__,
                "source": (e.text or "").strip(),
                "severity": "error",
                "tool": "python",
            }
        )
        return finish()

    # A real, registered module, so tools that look the module up (dataclasses with
    # `from __future__ import annotations`, typing.get_type_hints, pickle…) work normally.
    module = types.ModuleType("your_code")
    module.__file__ = "your-code.py"
    sys.modules["your_code"] = module
    ns = module.__dict__
    ns["__builtins__"] = builtins
    sys.stdout, sys.stderr = stdout, stderr
    try:
        run_limited(lambda: exec(user_code, ns))
    except BaseException as e:  # noqa: BLE001 — anything the learner's module raises on import
        report["setupErrors"].append({"file": "your-code", **describe(e)})
        sys.stdout = real_stdout
        return finish()

    tree = ast.parse(tests_src, "tests.py")
    tree = ast.fix_missing_locations(AssertRewriter(tests_src).visit(tree))
    ns.update({"raises": raises, "approx": approx, "_dts_cmp": _dts_cmp, "_dts_true": _dts_true})
    try:
        exec(compile(tree, "tests.py", "exec"), ns)
    except BaseException as e:  # noqa: BLE001
        report["setupErrors"].append({"file": "tests", **describe(e)})
        sys.stdout = real_stdout
        return finish()

    tests = [(n, f) for n, f in ns.items() if n.startswith("test_") and callable(f) and getattr(getattr(f, "__code__", None), "co_filename", "") == "tests.py"]
    looped = False
    for name, fn in tests:
        label = (inspect.getdoc(fn) or name[5:].replace("_", " ")).strip().splitlines()[0]
        if looped:
            report["results"].append({"name": label, "pass": False, "ms": 0, "error": {"message": SKIPPED_MSG}})
            continue
        started = time.perf_counter()
        try:
            run_limited(fn)
            report["results"].append({"name": label, "pass": True, "ms": (time.perf_counter() - started) * 1000})
        except BaseException as e:  # noqa: BLE001
            looped = looped or isinstance(e, DtsTimeout)
            report["results"].append({"name": label, "pass": False, "ms": (time.perf_counter() - started) * 1000, "error": describe(e)})

    sys.stdout = real_stdout
    finish()


if __name__ == "__main__":
    main()
