test("parses a warning", () => {
  expect(parseLogLine("12:04:55 [WARN] Disk almost full")).toEqual({
    time: "12:04:55",
    level: "WARN",
    message: "Disk almost full",
  });
});

test("keeps brackets inside the message", () => {
  expect(parseLogLine("08:00:00 [ERROR] Job [42] failed")).toEqual({
    time: "08:00:00",
    level: "ERROR",
    message: "Job [42] failed",
  });
});

test("unknown level → null", () => {
  expect(parseLogLine("12:05:01 [DEBUG] cache hit")).toBeNull();
});

test("wrong format → null", () => {
  expect(parseLogLine("garbage")).toBeNull();
  expect(parseLogLine("")).toBeNull();
  expect(parseLogLine("12:00:00 INFO no brackets")).toBeNull();
});

test("parses a whole log, skipping bad lines", () => {
  const log = "09:00:00 [INFO] Boot\n\n09:00:01 [TRACE] noise\n09:00:02 [ERROR] DB down";
  expect(parseLog(log)).toEqual([
    { time: "09:00:00", level: "INFO", message: "Boot" },
    { time: "09:00:02", level: "ERROR", message: "DB down" },
  ]);
});

test("an empty log has no entries", () => {
  expect(parseLog("")).toEqual([]);
});
