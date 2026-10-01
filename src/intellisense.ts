// Monaco only ships a language service for TypeScript. For C and Python, register lightweight
// providers: keywords, snippets, the standard library the challenges use, the test harness, and
// the identifiers declared in the learner's own code (completion, hover and signature help).
import type * as Monaco from "monaco-editor";

type M = typeof Monaco;

interface Fn {
  name: string;
  /** Full signature, e.g. "size_t strlen(const char *s)" */
  sig: string;
  params: string[];
  doc: string;
  /** Header (C) or module (Python) it comes from */
  from?: string;
}

interface LangData {
  keywords: string[];
  types?: string[];
  constants: string[];
  functions: Fn[];
  /** Methods offered after a "." (Python only; no type inference, so all common ones). */
  methods?: Fn[];
  snippets: { label: string; body: string; doc: string }[];
}

/* ---------------------------------------------------------------- C */

const cfn = (from: string, ret: string, name: string, params: string[], doc: string): Fn => ({
  name,
  from,
  params,
  sig: `${ret} ${name}(${params.join(", ")})`.trim(),
  doc,
});

const C: LangData = {
  keywords: [
    "auto", "break", "case", "const", "continue", "default", "do", "else", "enum", "extern", "for", "goto", "if",
    "inline", "register", "restrict", "return", "sizeof", "static", "struct", "switch", "typedef", "union",
    "volatile", "while", "_Alignof", "_Static_assert",
  ],
  types: [
    "void", "char", "short", "int", "long", "float", "double", "signed", "unsigned", "bool", "size_t", "ptrdiff_t",
    "int8_t", "int16_t", "int32_t", "int64_t", "uint8_t", "uint16_t", "uint32_t", "uint64_t", "intptr_t", "uintptr_t",
  ],
  constants: [
    "NULL", "true", "false", "INT_MAX", "INT_MIN", "UINT_MAX", "LONG_MAX", "LONG_MIN", "LLONG_MAX", "LLONG_MIN",
    "CHAR_BIT", "SIZE_MAX", "INT32_MAX", "INT32_MIN", "UINT32_MAX", "INT64_MAX", "UINT64_MAX", "UINT8_MAX", "EOF",
  ],
  functions: [
    cfn("string.h", "size_t", "strlen", ["const char *s"], "Length of `s`, not counting the terminating `'\\0'`."),
    cfn("string.h", "int", "strcmp", ["const char *a", "const char *b"], "Compares two strings. Returns 0 if equal, <0 if `a` sorts first, >0 otherwise."),
    cfn("string.h", "int", "strncmp", ["const char *a", "const char *b", "size_t n"], "Like `strcmp`, but compares at most `n` characters."),
    cfn("string.h", "char *", "strcpy", ["char *dst", "const char *src"], "Copies `src` (including `'\\0'`) into `dst`. `dst` must be big enough."),
    cfn("string.h", "char *", "strncpy", ["char *dst", "const char *src", "size_t n"], "Copies at most `n` chars. Does **not** terminate `dst` if `src` is too long."),
    cfn("string.h", "char *", "strcat", ["char *dst", "const char *src"], "Appends `src` to the end of `dst`."),
    cfn("string.h", "char *", "strchr", ["const char *s", "int c"], "Pointer to the first `c` in `s`, or `NULL`."),
    cfn("string.h", "char *", "strrchr", ["const char *s", "int c"], "Pointer to the last `c` in `s`, or `NULL`."),
    cfn("string.h", "char *", "strstr", ["const char *haystack", "const char *needle"], "Pointer to the first occurrence of `needle`, or `NULL`."),
    cfn("string.h", "char *", "strdup", ["const char *s"], "Heap-allocated copy of `s`. Release it with `free`."),
    cfn("string.h", "void *", "memcpy", ["void *dst", "const void *src", "size_t n"], "Copies `n` bytes. The regions must not overlap (use `memmove` if they might)."),
    cfn("string.h", "void *", "memmove", ["void *dst", "const void *src", "size_t n"], "Copies `n` bytes; safe when the regions overlap."),
    cfn("string.h", "void *", "memset", ["void *s", "int c", "size_t n"], "Fills the first `n` bytes of `s` with the byte `c`."),
    cfn("string.h", "int", "memcmp", ["const void *a", "const void *b", "size_t n"], "Compares `n` bytes. Returns 0 if equal."),
    cfn("stdlib.h", "void *", "malloc", ["size_t size"], "Allocates `size` uninitialised bytes. Returns `NULL` on failure. Release with `free`."),
    cfn("stdlib.h", "void *", "calloc", ["size_t count", "size_t size"], "Allocates `count * size` zeroed bytes. Returns `NULL` on failure."),
    cfn("stdlib.h", "void *", "realloc", ["void *ptr", "size_t size"], "Resizes an allocation. On failure returns `NULL` and leaves `ptr` untouched."),
    cfn("stdlib.h", "void", "free", ["void *ptr"], "Releases memory from `malloc`/`calloc`/`realloc`. `free(NULL)` is a no-op."),
    cfn("stdlib.h", "int", "abs", ["int n"], "Absolute value. `abs(INT_MIN)` is undefined behaviour."),
    cfn("stdlib.h", "long", "labs", ["long n"], "Absolute value of a `long`."),
    cfn("stdlib.h", "int", "atoi", ["const char *s"], "Parses a decimal integer. No error reporting; prefer `strtol`."),
    cfn("stdlib.h", "long", "strtol", ["const char *s", "char **end", "int base"], "Parses an integer in `base`; `*end` points past the last digit read."),
    cfn("stdlib.h", "void", "qsort", ["void *base", "size_t count", "size_t size", "int (*cmp)(const void *, const void *)"], "Sorts an array in place using the comparison function `cmp`."),
    cfn("stdlib.h", "void *", "bsearch", ["const void *key", "const void *base", "size_t count", "size_t size", "int (*cmp)(const void *, const void *)"], "Binary search in a sorted array. Returns a pointer to the match or `NULL`."),
    cfn("ctype.h", "int", "isalpha", ["int c"], "Non-zero if `c` is a letter."),
    cfn("ctype.h", "int", "isdigit", ["int c"], "Non-zero if `c` is `'0'`–`'9'`."),
    cfn("ctype.h", "int", "isalnum", ["int c"], "Non-zero if `c` is a letter or digit."),
    cfn("ctype.h", "int", "isspace", ["int c"], "Non-zero for space, `\\t`, `\\n`, `\\v`, `\\f`, `\\r`."),
    cfn("ctype.h", "int", "isupper", ["int c"], "Non-zero if `c` is an uppercase letter."),
    cfn("ctype.h", "int", "islower", ["int c"], "Non-zero if `c` is a lowercase letter."),
    cfn("ctype.h", "int", "ispunct", ["int c"], "Non-zero if `c` is punctuation."),
    cfn("ctype.h", "int", "toupper", ["int c"], "Uppercase version of `c` (unchanged if not a letter)."),
    cfn("ctype.h", "int", "tolower", ["int c"], "Lowercase version of `c` (unchanged if not a letter)."),
    cfn("stdio.h", "int", "printf", ["const char *format", "..."], "Prints formatted output. Shows up in the results panel's output."),
    cfn("stdio.h", "int", "snprintf", ["char *buf", "size_t size", "const char *format", "..."], "Formats into `buf`, writing at most `size` bytes including `'\\0'`."),
    cfn("stdio.h", "int", "puts", ["const char *s"], "Prints `s` followed by a newline."),
    cfn("stdio.h", "int", "putchar", ["int c"], "Prints one character."),
    cfn("harness", "", "TEST", ["name"], "Defines a test case: `TEST(name) { EXPECT_EQ(...); }`."),
    cfn("harness", "", "EXPECT_EQ", ["actual", "expected"], "Integers equal (compared as `long long`)."),
    cfn("harness", "", "EXPECT_NE", ["actual", "expected"], "Integers differ."),
    cfn("harness", "", "EXPECT_UEQ", ["actual", "expected"], "Unsigned integers equal (`unsigned long long`)."),
    cfn("harness", "", "EXPECT_TRUE", ["cond"], "Condition is true."),
    cfn("harness", "", "EXPECT_FALSE", ["cond"], "Condition is false."),
    cfn("harness", "", "EXPECT_NEAR", ["actual", "expected", "eps"], "Doubles within `eps` of each other."),
    cfn("harness", "", "EXPECT_STR_EQ", ["actual", "expected"], "C strings equal (NULL-safe)."),
    cfn("harness", "", "EXPECT_NULL", ["ptr"], "Pointer is `NULL`."),
    cfn("harness", "", "EXPECT_NOT_NULL", ["ptr"], "Pointer is not `NULL`."),
    cfn("harness", "", "EXPECT_PTR_EQ", ["actual", "expected"], "Pointers equal."),
    cfn("harness", "", "EXPECT_INT_ARRAY_EQ", ["actual", "expected", "n"], "`int` arrays of length `n` equal."),
  ],
  snippets: [
    { label: "for", body: "for (${1:size_t} ${2:i} = 0; ${2:i} < ${3:n}; ${2:i}++) {\n\t$0\n}", doc: "Counting for loop" },
    { label: "while", body: "while (${1:cond}) {\n\t$0\n}", doc: "While loop" },
    { label: "if", body: "if (${1:cond}) {\n\t$0\n}", doc: "If statement" },
    { label: "ifelse", body: "if (${1:cond}) {\n\t$2\n} else {\n\t$0\n}", doc: "If / else" },
    { label: "switch", body: "switch (${1:value}) {\ncase ${2:0}:\n\t$0\n\tbreak;\ndefault:\n\tbreak;\n}", doc: "Switch statement" },
    { label: "struct", body: "typedef struct {\n\t$0\n} ${1:Name};", doc: "typedef struct" },
    { label: "include", body: "#include <${1:stdlib.h}>", doc: "#include a header" },
    { label: "malloc", body: "${1:int} *${2:p} = malloc(${3:n} * sizeof *${2:p});\nif (!${2:p}) return ${4:NULL};", doc: "Allocate and check for NULL" },
  ],
};

/* ---------------------------------------------------------------- Python */

const pfn = (name: string, params: string[], doc: string, ret = "", from?: string): Fn => ({
  name,
  from,
  params,
  sig: `${name}(${params.join(", ")})${ret ? ` -> ${ret}` : ""}`,
  doc,
});

const PY: LangData = {
  keywords: [
    "and", "as", "assert", "async", "await", "break", "class", "continue", "def", "del", "elif", "else", "except",
    "finally", "for", "from", "global", "if", "import", "in", "is", "lambda", "match", "case", "nonlocal", "not",
    "or", "pass", "raise", "return", "try", "while", "with", "yield",
  ],
  constants: ["True", "False", "None", "self", "cls"],
  functions: [
    pfn("print", ["*values", "sep=' '", "end='\\n'"], "Prints values. Output shows up in the results panel."),
    pfn("len", ["obj"], "Number of items in a container.", "int"),
    pfn("range", ["start", "stop", "step=1"], "Sequence of integers from `start` up to (not including) `stop`.", "range"),
    pfn("enumerate", ["iterable", "start=0"], "Yields `(index, item)` pairs.", "enumerate"),
    pfn("zip", ["*iterables", "strict=False"], "Yields tuples pairing up items from each iterable.", "zip"),
    pfn("sorted", ["iterable", "key=None", "reverse=False"], "New sorted list.", "list"),
    pfn("reversed", ["seq"], "Iterator over `seq` backwards.", "Iterator"),
    pfn("sum", ["iterable", "start=0"], "Adds up the items.", "number"),
    pfn("min", ["iterable", "key=None", "default=..."], "Smallest item (or smallest argument)."),
    pfn("max", ["iterable", "key=None", "default=..."], "Largest item (or largest argument)."),
    pfn("abs", ["x"], "Absolute value."),
    pfn("round", ["number", "ndigits=None"], "Rounds to `ndigits` decimals (banker's rounding)."),
    pfn("divmod", ["a", "b"], "`(a // b, a % b)`.", "tuple"),
    pfn("any", ["iterable"], "True if any item is truthy.", "bool"),
    pfn("all", ["iterable"], "True if every item is truthy.", "bool"),
    pfn("map", ["func", "*iterables"], "Applies `func` to every item.", "map"),
    pfn("filter", ["func", "iterable"], "Items for which `func(item)` is truthy.", "filter"),
    pfn("isinstance", ["obj", "classinfo"], "True if `obj` is an instance of `classinfo` (a type or tuple of types).", "bool"),
    pfn("int", ["x=0", "base=10"], "Converts to an integer.", "int"),
    pfn("float", ["x=0.0"], "Converts to a float.", "float"),
    pfn("str", ["obj=''"], "String form of `obj`.", "str"),
    pfn("bool", ["x=False"], "Truthiness of `x`.", "bool"),
    pfn("list", ["iterable=()"], "New list.", "list"),
    pfn("dict", ["**kwargs"], "New dictionary.", "dict"),
    pfn("set", ["iterable=()"], "New set.", "set"),
    pfn("tuple", ["iterable=()"], "New tuple.", "tuple"),
    pfn("frozenset", ["iterable=()"], "New immutable set.", "frozenset"),
    pfn("iter", ["obj"], "Iterator over `obj`.", "Iterator"),
    pfn("next", ["iterator", "default=..."], "Next item, or `default` when exhausted."),
    pfn("ord", ["c"], "Unicode code point of a one-character string.", "int"),
    pfn("chr", ["i"], "One-character string for a code point.", "str"),
    pfn("repr", ["obj"], "Developer-facing string form.", "str"),
    pfn("hash", ["obj"], "Hash value.", "int"),
    pfn("getattr", ["obj", "name", "default=..."], "Attribute `name` of `obj`."),
    pfn("setattr", ["obj", "name", "value"], "Sets attribute `name` on `obj`."),
    pfn("hasattr", ["obj", "name"], "True if `obj` has attribute `name`.", "bool"),
    pfn("super", [], "Proxy for the parent class."),
    pfn("raises", ["exc_type"], "Test helper: `with raises(ValueError): ...`", "", "harness"),
    pfn("approx", ["expected", "rel=1e-6", "abs=1e-12"], "Test helper: `assert x == approx(1.5)`", "", "harness"),
  ],
  methods: [
    pfn("append", ["x"], "list: add `x` to the end.", "None", "list"),
    pfn("extend", ["iterable"], "list: append every item.", "None", "list"),
    pfn("insert", ["i", "x"], "list: insert `x` before index `i`.", "None", "list"),
    pfn("pop", ["i=-1"], "list/dict/set: remove and return an item.", "", "list"),
    pfn("remove", ["x"], "list/set: remove the first `x`.", "None", "list"),
    pfn("index", ["x"], "list/str: position of the first `x`.", "int", "list"),
    pfn("count", ["x"], "list/str: number of occurrences of `x`.", "int", "list"),
    pfn("sort", ["key=None", "reverse=False"], "list: sort in place.", "None", "list"),
    pfn("reverse", [], "list: reverse in place.", "None", "list"),
    pfn("copy", [], "list/dict/set: shallow copy.", "", "list"),
    pfn("clear", [], "list/dict/set: remove everything.", "None", "list"),
    pfn("get", ["key", "default=None"], "dict: value for `key`, or `default`.", "", "dict"),
    pfn("keys", [], "dict: view of the keys.", "", "dict"),
    pfn("values", [], "dict: view of the values.", "", "dict"),
    pfn("items", [], "dict: view of `(key, value)` pairs.", "", "dict"),
    pfn("setdefault", ["key", "default=None"], "dict: `d[key]`, inserting `default` first if missing.", "", "dict"),
    pfn("update", ["other"], "dict/set: merge in `other`.", "None", "dict"),
    pfn("add", ["x"], "set: add `x`.", "None", "set"),
    pfn("discard", ["x"], "set: remove `x` if present.", "None", "set"),
    pfn("union", ["*others"], "set: items in any.", "set", "set"),
    pfn("intersection", ["*others"], "set: items in all.", "set", "set"),
    pfn("difference", ["*others"], "set: items not in the others.", "set", "set"),
    pfn("split", ["sep=None", "maxsplit=-1"], "str: list of the words, split on `sep` (whitespace by default).", "list[str]", "str"),
    pfn("join", ["iterable"], "str: concatenate strings with this one between them.", "str", "str"),
    pfn("strip", ["chars=None"], "str: remove leading and trailing whitespace (or `chars`).", "str", "str"),
    pfn("lstrip", ["chars=None"], "str: remove leading whitespace.", "str", "str"),
    pfn("rstrip", ["chars=None"], "str: remove trailing whitespace.", "str", "str"),
    pfn("lower", [], "str: lowercase copy.", "str", "str"),
    pfn("upper", [], "str: uppercase copy.", "str", "str"),
    pfn("title", [], "str: Title Case copy.", "str", "str"),
    pfn("capitalize", [], "str: first letter uppercase.", "str", "str"),
    pfn("replace", ["old", "new", "count=-1"], "str: copy with `old` replaced by `new`.", "str", "str"),
    pfn("startswith", ["prefix"], "str: True if it starts with `prefix`.", "bool", "str"),
    pfn("endswith", ["suffix"], "str: True if it ends with `suffix`.", "bool", "str"),
    pfn("find", ["sub"], "str: index of `sub`, or -1.", "int", "str"),
    pfn("format", ["*args", "**kwargs"], "str: fill `{}` placeholders.", "str", "str"),
    pfn("isdigit", [], "str: True if all characters are digits.", "bool", "str"),
    pfn("isalpha", [], "str: True if all characters are letters.", "bool", "str"),
    pfn("isalnum", [], "str: True if all characters are letters or digits.", "bool", "str"),
    pfn("isspace", [], "str: True if all characters are whitespace.", "bool", "str"),
    pfn("splitlines", [], "str: list of lines.", "list[str]", "str"),
    pfn("partition", ["sep"], "str: `(before, sep, after)`.", "tuple", "str"),
    pfn("zfill", ["width"], "str: pad with zeros on the left.", "str", "str"),
  ],
  snippets: [
    { label: "def", body: "def ${1:name}(${2:args}) -> ${3:None}:\n\t${0:pass}", doc: "Function definition" },
    { label: "class", body: "class ${1:Name}:\n\tdef __init__(self${2:, args}) -> None:\n\t\t${0:pass}", doc: "Class definition" },
    { label: "for", body: "for ${1:item} in ${2:items}:\n\t${0:pass}", doc: "For loop" },
    { label: "fori", body: "for ${1:i} in range(${2:n}):\n\t${0:pass}", doc: "For loop over range" },
    { label: "while", body: "while ${1:cond}:\n\t${0:pass}", doc: "While loop" },
    { label: "if", body: "if ${1:cond}:\n\t${0:pass}", doc: "If statement" },
    { label: "ifelse", body: "if ${1:cond}:\n\t${2:pass}\nelse:\n\t${0:pass}", doc: "If / else" },
    { label: "try", body: "try:\n\t${1:pass}\nexcept ${2:Exception} as ${3:e}:\n\t${0:raise}", doc: "Try / except" },
    { label: "with", body: "with ${1:expr} as ${2:name}:\n\t${0:pass}", doc: "With statement" },
    { label: "main", body: 'if __name__ == "__main__":\n\t${0:main()}', doc: "Script entry point" },
  ],
};

/* ---------------------------------------------------------------- learner's own code */

interface Declared {
  name: string;
  kind: "function" | "variable" | "type" | "constant";
  fn?: Fn;
}

const C_RESERVED = new Set([...C.keywords, ...(C.types ?? [])]);
// "return x;" or "case X:" look like declarations; a real declaration's first word is a type.
const C_KEYWORDS = new Set(C.keywords.filter((k) => !["const", "static", "struct", "enum", "union", "volatile", "register", "extern", "unsigned", "signed"].includes(k)));
const C_CONSTANTS = new Set(C.constants);

function cDeclarations(code: string): Declared[] {
  const out = new Map<string, Declared>();
  const src = code.replace(/\/\*[\s\S]*?\*\/|\/\/.*$/gm, "");
  // Function definitions/prototypes: "<type> name(params)" at the start of a line.
  for (const m of src.matchAll(/^[ \t]*((?:[A-Za-z_]\w*[\s*]+)+?)\**([A-Za-z_]\w*)\s*\(([^)]*)\)\s*[{;]/gm)) {
    const [, ret, name, rawParams] = m;
    if (C_RESERVED.has(name) || /\b(return|else)\b/.test(ret)) continue;
    const params = rawParams.trim() === "void" || !rawParams.trim() ? [] : rawParams.split(",").map((p) => p.trim().replace(/\s+/g, " "));
    const sig = `${ret.trim().replace(/\s+/g, " ")} ${name}(${params.join(", ")})`;
    out.set(name, { name, kind: "function", fn: { name, sig, params, doc: "Defined in your code." } });
  }
  for (const m of src.matchAll(/#define\s+([A-Za-z_]\w*)/g)) out.set(m[1], { name: m[1], kind: "constant" });
  for (const m of src.matchAll(/\b(?:struct|enum|union)\s+([A-Za-z_]\w*)/g)) if (!out.has(m[1])) out.set(m[1], { name: m[1], kind: "type" });
  for (const m of src.matchAll(/}\s*([A-Za-z_]\w*)\s*;/g)) if (!out.has(m[1])) out.set(m[1], { name: m[1], kind: "type" });
  // Variables and parameters: "type name" / "type *name" followed by = , ; ) or [.
  for (const m of src.matchAll(/\b([A-Za-z_]\w*)[\s*]+\**([A-Za-z_]\w*)\s*(?=[=,;)[])/g)) {
    const [, type, name] = m;
    if (C_KEYWORDS.has(type) || C_RESERVED.has(name) || C_CONSTANTS.has(name) || out.has(name)) continue;
    out.set(name, { name, kind: "variable" });
  }
  return [...out.values()];
}

export const PY_RESERVED = new Set([...PY.keywords, ...PY.constants]);

function pyDeclarations(code: string): Declared[] {
  const out = new Map<string, Declared>();
  const src = code.replace(/#.*$/gm, "");
  for (const m of src.matchAll(/^[ \t]*(?:async\s+)?def\s+([A-Za-z_]\w*)\s*\(([^)]*)\)\s*(?:->\s*([^:]+))?:/gm)) {
    const [, name, rawParams, ret] = m;
    const params = rawParams.split(",").map((p) => p.trim().replace(/\s+/g, " ")).filter((p) => p && p !== "self" && p !== "cls");
    out.set(name, { name, kind: "function", fn: { name, params, sig: `${name}(${params.join(", ")})${ret ? ` -> ${ret.trim()}` : ""}`, doc: "Defined in your code." } });
  }
  for (const m of src.matchAll(/^[ \t]*class\s+([A-Za-z_]\w*)/gm)) out.set(m[1], { name: m[1], kind: "type" });
  for (const m of src.matchAll(/^[ \t]*(?:from\s+[\w.]+\s+)?import\s+([\w., ]+)/gm))
    for (const part of m[1].split(",")) {
      const name = part.trim().split(/\s+as\s+/).pop()!.trim();
      if (name && !out.has(name)) out.set(name, { name, kind: "variable" });
    }
  const addVar = (name: string) => {
    name = name.trim().replace(/^\*|^self\./, "");
    if (/^[A-Za-z_]\w*$/.test(name) && !out.has(name) && !PY_RESERVED.has(name)) out.set(name, { name, kind: "variable" });
  };
  // Assignments at the start of a line (incl. "a, b = ..." and "self.x: int = ..."), and walrus.
  for (const m of src.matchAll(/^[ \t]*([\w., *]+?)\s*(?::[^=\n]+)?(?:[-+*/%|&]|\/\/)?=(?!=)/gm)) m[1].split(",").forEach(addVar);
  for (const m of src.matchAll(/\b([A-Za-z_]\w*)\s*:=/g)) addVar(m[1]);
  // Function parameters.
  for (const m of src.matchAll(/\bdef\s+\w+\s*\(([^)]*)\)/g)) for (const p of m[1].split(",")) addVar(p.split(/[:=]/)[0]);
  for (const m of src.matchAll(/\bfor\s+([\w, ]+?)\s+in\b/g))
    m[1].split(",").forEach(addVar);
  return [...out.values()];
}

/* ---------------------------------------------------------------- providers */

/** Parameter positions inside the signature, so Monaco can highlight the active one. */
function paramOffsets(fn: Fn): { label: [number, number] }[] {
  let at = fn.sig.indexOf("(") + 1;
  return fn.params.map((p) => {
    const start = fn.sig.indexOf(p, at);
    at = start + p.length;
    return { label: [start, at] as [number, number] };
  });
}

function fnDoc(fn: Fn) {
  return { value: "```\n" + fn.sig + "\n```\n" + fn.doc + (fn.from ? `\n\n*${fn.from}*` : "") };
}

function register(monaco: M, lang: "c" | "python", data: LangData, declarations: (code: string) => Declared[]) {
  const K = monaco.languages.CompletionItemKind;
  const lookup = (model: Monaco.editor.ITextModel, name: string, afterDot: boolean) =>
    (afterDot ? data.methods : undefined)?.find((f) => f.name === name) ??
    declarations(model.getValue()).find((d) => d.name === name)?.fn ??
    data.functions.find((f) => f.name === name) ??
    data.methods?.find((f) => f.name === name);

  monaco.languages.registerCompletionItemProvider(lang, {
    triggerCharacters: lang === "python" ? ["."] : [],
    provideCompletionItems(model, position) {
      const word = model.getWordUntilPosition(position);
      const range = new monaco.Range(position.lineNumber, word.startColumn, position.lineNumber, word.endColumn);
      const before = model.getLineContent(position.lineNumber).slice(0, word.startColumn - 1);
      const items: Monaco.languages.CompletionItem[] = [];
      const fnItem = (f: Fn, kind: Monaco.languages.CompletionItemKind, sort: string): Monaco.languages.CompletionItem => ({
        label: { label: f.name, detail: `(${f.params.join(", ")})`, description: f.from },
        kind,
        insertText: f.name,
        detail: f.sig,
        documentation: { value: f.doc },
        range,
        sortText: sort + f.name,
      });

      if (lang === "python" && /\.\s*$/.test(before)) {
        for (const f of data.methods ?? []) items.push(fnItem(f, K.Method, "0"));
        return { suggestions: items };
      }
      if (lang === "c" && /(\.|->)\s*$/.test(before)) {
        // No type info for struct members: offer the identifiers already written in this file.
        const seen = new Set<string>();
        for (const m of model.getValue().matchAll(/[A-Za-z_]\w*/g)) if (!seen.has(m[0]) && !C_RESERVED.has(m[0])) seen.add(m[0]);
        seen.delete(word.word);
        for (const w of seen) items.push({ label: w, kind: K.Field, insertText: w, range });
        return { suggestions: items };
      }
      if (lang === "c" && /#\s*include\s*<[\w./]*$/.test(model.getLineContent(position.lineNumber).slice(0, position.column - 1))) {
        for (const h of ["stdio.h", "stdlib.h", "string.h", "ctype.h", "stdbool.h", "stddef.h", "stdint.h", "limits.h", "math.h"])
          items.push({ label: h, kind: K.File, insertText: h, range });
        return { suggestions: items };
      }

      for (const d of declarations(model.getValue())) {
        if (d.name === word.word) continue;
        if (d.fn) items.push(fnItem(d.fn, K.Function, "0"));
        else
          items.push({
            label: d.name,
            kind: d.kind === "type" ? K.Struct : d.kind === "constant" ? K.Constant : K.Variable,
            insertText: d.name,
            range,
            sortText: "0" + d.name,
          });
      }
      for (const f of data.functions) items.push(fnItem(f, K.Function, "1"));
      for (const k of data.keywords) items.push({ label: k, kind: K.Keyword, insertText: k, range, sortText: "2" + k });
      for (const t of data.types ?? []) items.push({ label: t, kind: K.TypeParameter, insertText: t, range, sortText: "2" + t });
      for (const c of data.constants) items.push({ label: c, kind: K.Constant, insertText: c, range, sortText: "2" + c });
      for (const s of data.snippets)
        items.push({
          label: s.label,
          kind: K.Snippet,
          insertText: s.body,
          insertTextRules: monaco.languages.CompletionItemInsertTextRule.InsertAsSnippet,
          detail: s.doc,
          documentation: { value: "```\n" + s.body.replace(/\$\{\d+:([^}]*)\}/g, "$1").replace(/\$\d/g, "") + "\n```" },
          range,
          sortText: "3" + s.label,
        });
      return { suggestions: items };
    },
  });

  monaco.languages.registerHoverProvider(lang, {
    provideHover(model, position) {
      const word = model.getWordAtPosition(position);
      if (!word) return null;
      const afterDot = /(\.|->)\s*$/.test(model.getLineContent(position.lineNumber).slice(0, word.startColumn - 1));
      const fn = lookup(model, word.word, afterDot);
      if (!fn) return null;
      return {
        range: new monaco.Range(position.lineNumber, word.startColumn, position.lineNumber, word.endColumn),
        contents: [fnDoc(fn)],
      };
    },
  });

  monaco.languages.registerSignatureHelpProvider(lang, {
    signatureHelpTriggerCharacters: ["(", ","],
    signatureHelpRetriggerCharacters: [","],
    provideSignatureHelp(model, position) {
      // Walk back from the cursor to the unclosed "(" and count top-level commas on the way.
      const text = model.getValueInRange(new monaco.Range(Math.max(1, position.lineNumber - 20), 1, position.lineNumber, position.column));
      let depth = 0;
      let commas = 0;
      let open = -1;
      for (let i = text.length - 1; i >= 0; i--) {
        const ch = text[i];
        if (ch === ")" || ch === "]" || ch === "}") depth++;
        else if (ch === "(" || ch === "[" || ch === "{") {
          if (depth === 0) {
            if (ch === "(") open = i;
            break;
          }
          depth--;
        } else if (ch === "," && depth === 0) commas++;
      }
      if (open < 0) return null;
      const m = /([A-Za-z_]\w*)\s*$/.exec(text.slice(0, open));
      if (!m) return null;
      const afterDot = /(\.|->)\s*$/.test(text.slice(0, m.index));
      const fn = lookup(model, m[1], afterDot);
      if (!fn) return null;
      return {
        value: {
          signatures: [
            {
              label: fn.sig,
              documentation: { value: fn.doc },
              parameters: paramOffsets(fn),
            },
          ],
          activeSignature: 0,
          activeParameter: Math.min(commas, Math.max(0, fn.params.length - 1)),
        },
        dispose() {},
      };
    },
  });
}

export function registerIntellisense(monaco: M) {
  register(monaco, "c", C, cDeclarations);
  register(monaco, "python", PY, pyDeclarations);
}
