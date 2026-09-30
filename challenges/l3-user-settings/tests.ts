test("nothing set → all defaults", () => {
  expect(resolveSettings({})).toEqual({ theme: "light", fontSize: 14, emailAlerts: true });
});

test("keeps the values that were set", () => {
  expect(resolveSettings({ theme: "dark", fontSize: 18 })).toEqual({ theme: "dark", fontSize: 18, emailAlerts: true });
});

test("false is a real choice, not 'unset'", () => {
  expect(resolveSettings({ emailAlerts: false })).toEqual({ theme: "light", fontSize: 14, emailAlerts: false });
});

test("everything set → nothing changes", () => {
  const all: UserSettings = { theme: "dark", fontSize: 12, emailAlerts: false };
  expect(resolveSettings(all)).toEqual({ theme: "dark", fontSize: 12, emailAlerts: false });
});

test("an explicit undefined also gets the default", () => {
  expect(resolveSettings({ fontSize: undefined }).fontSize).toBe(14);
});
