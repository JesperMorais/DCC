interface Settings {
  readonly userId: string;
  apiToken: string;
  theme?: "light" | "dark";
  fontSize?: number;
  autosave?: boolean;
}

const DEFAULTS = { theme: "light", fontSize: 14, autosave: true } as const;

type Resolved<T> = any;
type SettingsPatch = any;
type PublicSettings = any;

function resolve(settings: Settings): Resolved<Settings> {
  return settings;
}

function applyPatch(settings: Settings, patch: SettingsPatch): Settings {
  return settings;
}

function toPublic(settings: Resolved<Settings>): PublicSettings {
  return settings;
}
