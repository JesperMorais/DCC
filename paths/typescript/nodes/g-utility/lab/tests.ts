const ada: Settings = { userId: "u1", apiToken: "secret-123", fontSize: 18 };

type cases = [
  Expect<
    Equal<
      Resolved<Settings>,
      {
        readonly userId: string;
        readonly apiToken: string;
        readonly theme: "light" | "dark";
        readonly fontSize: number;
        readonly autosave: boolean;
      }
    >
  >,
  Expect<Equal<Resolved<{ a?: number; readonly b: string }>, { readonly a: number; readonly b: string }>>,
  Expect<Equal<keyof SettingsPatch, "apiToken" | "theme" | "fontSize" | "autosave">>,
  Expect<Equal<SettingsPatch, { apiToken?: string; theme?: "light" | "dark"; fontSize?: number; autosave?: boolean }>>,
  Expect<Equal<keyof PublicSettings, "userId" | "theme" | "fontSize" | "autosave">>,
  Expect<Equal<PublicSettings["fontSize"], number>>,
];

// @ts-expect-error — userId can't be patched
applyPatch(ada, { userId: "u2" });

// @ts-expect-error — a resolved settings object is readonly
resolve(ada).theme = "dark";

test("resolve fills missing fields from DEFAULTS", () => {
  expect(resolve(ada)).toEqual({ userId: "u1", apiToken: "secret-123", theme: "light", fontSize: 18, autosave: true });
});

test("resolve keeps saved values, including false", () => {
  const saved: Settings = { userId: "u2", apiToken: "t", theme: "dark", fontSize: 12, autosave: false };
  expect(resolve(saved)).toEqual({ userId: "u2", apiToken: "t", theme: "dark", fontSize: 12, autosave: false });
});

test("resolve treats an explicit undefined as missing", () => {
  expect(resolve({ userId: "u3", apiToken: "t", theme: undefined }).theme).toBe("light");
});

test("applyPatch replaces fields and returns a new object", () => {
  const next = applyPatch(ada, { theme: "dark", autosave: false });
  expect(next).toEqual({ userId: "u1", apiToken: "secret-123", fontSize: 18, theme: "dark", autosave: false });
  expect(ada).toEqual({ userId: "u1", apiToken: "secret-123", fontSize: 18 });
});

test("applyPatch ignores undefined values in the patch", () => {
  const next = applyPatch(ada, { fontSize: undefined, theme: "dark" });
  expect(next.fontSize).toBe(18);
  expect(next.theme).toBe("dark");
});

test("toPublic really removes apiToken", () => {
  const resolved = resolve(ada);
  const pub = toPublic(resolved);
  expect(pub).toEqual({ userId: "u1", theme: "light", fontSize: 18, autosave: true });
  expect("apiToken" in pub).toBe(false);
  expect(resolved.apiToken).toBe("secret-123");
});
