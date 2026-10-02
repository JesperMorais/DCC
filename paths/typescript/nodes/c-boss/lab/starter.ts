// Make this a union of the four HTTP methods.
type Method = string;

type StatusClass = "2xx" | "3xx" | "4xx" | "5xx";

interface LogRequest {
  time: string;
  method: Method;
  path: string;
  status: number;
  ms: number;
}

interface AnalyseOptions {
  ignorePaths?: readonly string[];
}

interface Report {
  requests: number;
  unreadable: number[];
  byStatus: Record<StatusClass, number>;
  distinctPaths: number;
  slowest: [path: string, ms: number] | null;
  averageMs: string;
}

function statusClass(status: number): StatusClass {
  return "2xx";
}

function parseRequest(line: string): LogRequest | null {
  return null;
}

function analyse(text: string, options: AnalyseOptions = {}): Report {
  return {
    requests: 0,
    unreadable: [],
    byStatus: { "2xx": 0, "3xx": 0, "4xx": 0, "5xx": 0 },
    distinctPaths: 0,
    slowest: null,
    averageMs: "n/a",
  };
}
