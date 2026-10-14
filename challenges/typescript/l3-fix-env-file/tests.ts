test("reads simple settings", () => {
  expect(parseEnvFile("PORT=8080\nDEBUG=true")).toEqual({ PORT: "8080", DEBUG: "true" });
});

test("skips comments and blank lines and trims spaces", () => {
  expect(parseEnvFile("# server\n\nHOST = localhost")).toEqual({ HOST: "localhost" });
});

test("an empty file has no settings", () => {
  expect(parseEnvFile("")).toEqual({});
});

test("keeps an = inside the value", () => {
  expect(parseEnvFile("DB_URL=postgres://db/app?ssl=true")).toEqual({
    DB_URL: "postgres://db/app?ssl=true",
  });
});

test("a value made of several = signs", () => {
  const settings = parseEnvFile("NAME=demo\nSECRET=abc==\nMODE=a=b=c");
  expect(settings.NAME).toBe("demo");
  expect(settings.SECRET).toBe("abc==");
  expect(settings.MODE).toBe("a=b=c");
});

test("an empty value", () => {
  expect(parseEnvFile("API_KEY=")).toEqual({ API_KEY: "" });
});
