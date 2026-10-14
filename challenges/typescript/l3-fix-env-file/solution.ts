function parseEnvFile(text: string): Record<string, string> {
  const settings: Record<string, string> = {};
  for (const rawLine of text.split("\n")) {
    const line = rawLine.trim();
    if (line === "" || line.startsWith("#")) {
      continue;
    }
    const [key, ...rest] = line.split("=");
    settings[key.trim()] = rest.join("=").trim();
  }
  return settings;
}
