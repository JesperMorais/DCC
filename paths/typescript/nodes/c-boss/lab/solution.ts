type Method = "GET" | "POST" | "PUT" | "DELETE";

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
  if (status < 300) return "2xx";
  if (status < 400) return "3xx";
  if (status < 500) return "4xx";
  return "5xx";
}

const REQUEST_LINE = /^(\d\d:\d\d:\d\d) (GET|POST|PUT|DELETE) (\/\S*) ([2-5]\d\d) (\d+)ms$/;

function parseRequest(line: string): LogRequest | null {
  const m = line.trim().match(REQUEST_LINE);
  if (!m) return null;
  const [, time, method, path, status, ms] = m;
  // The regex only lets the four methods through.
  return { time, method: method as Method, path, status: Number(status), ms: Number(ms) };
}

function analyse(text: string, options: AnalyseOptions = {}): Report {
  const ignored = new Set(options.ignorePaths ?? []);
  const unreadable: number[] = [];
  const byStatus: Record<StatusClass, number> = { "2xx": 0, "3xx": 0, "4xx": 0, "5xx": 0 };
  const paths = new Set<string>();
  let slowest: Report["slowest"] = null;
  let requests = 0;
  let totalMs = 0;

  for (const [i, raw] of text.split("\n").entries()) {
    const line = raw.trim();
    if (line === "") continue;
    const req = parseRequest(line);
    if (!req) {
      unreadable.push(i + 1);
      continue;
    }
    if (ignored.has(req.path)) continue;
    requests++;
    totalMs += req.ms;
    paths.add(req.path);
    byStatus[statusClass(req.status)]++;
    if (slowest === null || req.ms > slowest[1]) slowest = [req.path, req.ms];
  }

  return {
    requests,
    unreadable,
    byStatus,
    distinctPaths: paths.size,
    slowest,
    averageMs: requests === 0 ? "n/a" : (totalMs / requests).toFixed(1),
  };
}
