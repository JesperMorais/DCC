const tick = (ms: number) => new Promise<void>((resolve) => setTimeout(resolve, ms));

test("an empty list gives an empty report", async () => {
  expect(await uploadAllFiles([], async () => {})).toEqual({ uploaded: [], failed: [] });
});

test("starts with the first file", async () => {
  const started: string[] = [];
  await uploadAllFiles(["a.jpg", "b.jpg"], async (name) => {
    started.push(name);
  });
  expect(started[0]).toBe("a.jpg");
});

test("reports every successful upload", async () => {
  const report = await uploadAllFiles(["a.jpg", "b.jpg"], () => tick(1));
  expect(report).toEqual({ uploaded: ["a.jpg", "b.jpg"], failed: [] });
});

test("a failure is reported and doesn't stop the rest", async () => {
  const report = await uploadAllFiles(["a.jpg", "huge.raw", "c.jpg"], async (name) => {
    await tick(1);
    if (name.endsWith(".raw")) throw new Error("too large");
  });
  expect(report).toEqual({ uploaded: ["a.jpg", "c.jpg"], failed: ["huge.raw"] });
});

test("uploads one at a time, in order", async () => {
  const events: string[] = [];
  await uploadAllFiles(["a", "b", "c"], async (name) => {
    events.push(`start ${name}`);
    await tick(name === "a" ? 5 : 1);
    events.push(`end ${name}`);
  });
  expect(events).toEqual(["start a", "end a", "start b", "end b", "start c", "end c"]);
});
