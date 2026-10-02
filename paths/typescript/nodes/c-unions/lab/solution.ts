type ShortTime = { minutes: number; seconds: number };
type LongTime = { hours: number; minutes: number };

type Preset = "soft-egg" | "hard-egg" | "pasta";

type TimerInput = number | Preset | ShortTime | LongTime;

function presetSeconds(preset: Preset): number {
  switch (preset) {
    case "soft-egg":
      return 360;
    case "hard-egg":
      return 600;
    case "pasta":
      return 540;
  }
}

function timerSeconds(input: TimerInput): number {
  if (typeof input === "number") return input;
  if (typeof input === "string") return presetSeconds(input);
  if ("hours" in input) return input.hours * 3600 + input.minutes * 60;
  return input.minutes * 60 + input.seconds;
}

function countdownText(remaining: number | null): string {
  if (remaining === null) return "--:--";
  const minutes = Math.floor(remaining / 60);
  const seconds = remaining % 60;
  return `${minutes}:${String(seconds).padStart(2, "0")}`;
}
