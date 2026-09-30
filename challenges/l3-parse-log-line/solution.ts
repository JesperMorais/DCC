type LogLevel = "INFO" | "WARN" | "ERROR";

interface LogEntry {
  time: string;
  level: LogLevel;
  message: string;
}

function parseLogLine(line: string): LogEntry | null {
  const m = line.match(/^(\S+) \[(\w+)\] (.+)$/);
  if (!m) return null;
  const [, time, level, message] = m;
  if (level === "INFO" || level === "WARN" || level === "ERROR") {
    return { time, level, message };
  }
  return null;
}

function parseLog(text: string): LogEntry[] {
  return text
    .split("\n")
    .map(parseLogLine)
    .filter((entry): entry is LogEntry => entry !== null);
}
