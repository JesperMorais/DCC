type LogLevel = "INFO" | "WARN" | "ERROR";

interface LogEntry {
  time: string;
  level: LogLevel;
  message: string;
}

function parseLogLine(line: string): LogEntry | null {
  return null;
}

function parseLog(text: string): LogEntry[] {
  return [];
}
