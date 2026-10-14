function parseEnvFile(text: string): Record<string, string> {
  const settings: Record<string, string> = {};
  for (const rawLine of text.split("\n")) {
    const line = rawLine.trim();
    if (line === "" || line.startsWith("#")) {
      continue;
    }
    const [key, value] = line.split("=");
    settings[key.trim()] = value.trim();
  }
  return settings;
}
