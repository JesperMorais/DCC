type ShortTime = { minutes: number; seconds: number };
type LongTime = { hours: number; minutes: number };

type Preset = string;

type TimerInput = any;

function timerSeconds(input: TimerInput): number {
  return 0;
}

function countdownText(remaining: number | null): string {
  return "";
}
