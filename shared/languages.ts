// The language registry, shared by the server and the UI.
export type Lang = "typescript" | "python" | "c";
export type Experience = "new" | "other-lang" | "js" | "some-ts" | "pro";

export interface LanguageInfo {
  id: Lang;
  name: string;
  /** Short badge text */
  short: string;
  ext: string;
  monaco: string;
  /** Badge colours (fixed brand-ish colours, readable on both themes) */
  badge: { bg: string; fg: string };
  /** Level names 1..6 (index 0 unused) */
  levels: [string, string, string, string, string, string, string];
  /** Onboarding options: the stored keys are shared, the wording is per language. */
  experience: Record<Experience, { title: string; body: string }>;
  /** What the "compile" step is called in results. */
  checkLabel: string;
  tagline: string;
  tessIntro: string;
}

export const LEVEL_BANDS: Record<number, readonly [number, number]> = {
  1: [700, 850],
  2: [850, 1000],
  3: [1000, 1150],
  4: [1150, 1300],
  5: [1300, 1450],
  6: [1450, 1650],
};

export const LANGUAGES: Record<Lang, LanguageInfo> = {
  typescript: {
    id: "typescript",
    name: "TypeScript",
    short: "TS",
    ext: "ts",
    monaco: "typescript",
    badge: { bg: "#3178c6", fg: "#ffffff" },
    levels: ["", "First steps", "Building blocks", "Shaping data", "Generics", "Advanced patterns", "Type wizardry"],
    experience: {
      new: { title: "Brand new to coding", body: "I've never really written code. Start from the very beginning." },
      "other-lang": { title: "I know another language", body: "Python, C#, Java… I get loops and functions, TypeScript is new." },
      js: { title: "I write JavaScript", body: "Comfortable with JS. Types and interfaces are the new part." },
      "some-ts": { title: "I use some TypeScript", body: "I've shipped TS, but generics and advanced types are fuzzy." },
      pro: { title: "TypeScript regular", body: "Bring on async patterns, conditional types and infer." },
    },
    checkLabel: "Type errors",
    tagline: "Strict mode, generics and a type system that catches bugs before they run.",
    tessIntro: "Pangolins are covered in scales, and your code gets covered in types. Both are armour.",
  },
  python: {
    id: "python",
    name: "Python",
    short: "Py",
    ext: "py",
    monaco: "python",
    badge: { bg: "#ffd43b", fg: "#1f3b57" },
    levels: ["", "First steps", "Building blocks", "Data structures", "Functions & classes", "Pythonic patterns", "Expert Python"],
    experience: {
      new: { title: "Brand new to coding", body: "I've never really written code. Start from the very beginning." },
      "other-lang": { title: "I know another language", body: "I can code, but Python's syntax and idioms are new to me." },
      js: { title: "I've written some Python", body: "Scripts and small tools. Comprehensions and classes are hazy." },
      "some-ts": { title: "I use Python regularly", body: "Comfortable day to day. Generators and decorators, less so." },
      pro: { title: "Python regular", body: "Give me protocols, closures, itertools and algorithms." },
    },
    checkLabel: "Syntax & type hints",
    tagline: "Readable, idiomatic Python with type hints. Tested pytest-style, the way teams ship it.",
    tessIntro: "Python reads almost like English. Let's make yours read like good English.",
  },
  c: {
    id: "c",
    name: "C",
    short: "C",
    ext: "c",
    monaco: "c",
    badge: { bg: "#5c6bc0", fg: "#ffffff" },
    levels: ["", "First steps", "Loops & arrays", "Strings & chars", "Pointers", "Memory & structs", "Systems craft"],
    experience: {
      new: { title: "Brand new to coding", body: "I've never really written code. Start from the very beginning." },
      "other-lang": { title: "I know a higher-level language", body: "Python, JS, Java… C's manual memory and pointers are new." },
      js: { title: "I've written a bit of C", body: "Loops and arrays are fine. Pointers still make me nervous." },
      "some-ts": { title: "I use C sometimes", body: "Pointers are OK. malloc/free and structs need practice." },
      pro: { title: "C regular", body: "Bit tricks, function pointers, hash tables. Let's go." },
    },
    checkLabel: "Compiler",
    tagline: "Close to the metal: arrays, pointers and memory, checked by AddressSanitizer.",
    tessIntro: "C hands you the raw scales. You build the armour yourself, and I'll check every plate.",
  },
};

export const LANG_IDS = Object.keys(LANGUAGES) as Lang[];
export const isLang = (s: string | undefined): s is Lang => !!s && s in LANGUAGES;
