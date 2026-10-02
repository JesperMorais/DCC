type timerCases = [
  Expect<Equal<Preset, "soft-egg" | "hard-egg" | "pasta">>,
  Expect<Equal<TimerInput, number | Preset | ShortTime | LongTime>>,
];

void (() => {
  // @ts-expect-error — "spaghetti" is not a preset
  timerSeconds("spaghetti");

  // @ts-expect-error — a ShortTime needs seconds, a LongTime needs hours
  timerSeconds({ minutes: 3 });
});

test("a number is already seconds", () => {
  expect(timerSeconds(90)).toBe(90);
  expect(timerSeconds(0)).toBe(0);
});

test("presets have fixed lengths", () => {
  expect(timerSeconds("soft-egg")).toBe(360);
  expect(timerSeconds("hard-egg")).toBe(600);
  expect(timerSeconds("pasta")).toBe(540);
});

test("minutes and seconds", () => {
  expect(timerSeconds({ minutes: 3, seconds: 30 })).toBe(210);
  expect(timerSeconds({ minutes: 0, seconds: 45 })).toBe(45);
});

test("hours and minutes", () => {
  expect(timerSeconds({ hours: 1, minutes: 15 })).toBe(4500);
  expect(timerSeconds({ hours: 0, minutes: 2 })).toBe(120);
});

test("no timer shows dashes", () => {
  expect(countdownText(null)).toBe("--:--");
});

test("seconds are padded to two digits", () => {
  expect(countdownText(65)).toBe("1:05");
  expect(countdownText(600)).toBe("10:00");
  expect(countdownText(59)).toBe("0:59");
});

test("zero is a finished timer, not a missing one", () => {
  expect(countdownText(0)).toBe("0:00");
});

test("minutes can go above 59", () => {
  expect(countdownText(4500)).toBe("75:00");
});
