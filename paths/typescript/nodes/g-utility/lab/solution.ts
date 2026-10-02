interface Settings {
  readonly userId: string;
  apiToken: string;
  theme?: "light" | "dark";
  fontSize?: number;
  autosave?: boolean;
}

const DEFAULTS = { theme: "light", fontSize: 14, autosave: true } as const;

type Resolved<T> = { readonly [K in keyof T]-?: T[K] };
type SettingsPatch = Partial<Omit<Settings, "userId">>;
type PublicSettings = Omit<Resolved<Settings>, "apiToken">;

function resolve(settings: Settings): Resolved<Settings> {
  return {
    ...settings,
    theme: settings.theme ?? DEFAULTS.theme,
    fontSize: settings.fontSize ?? DEFAULTS.fontSize,
    autosave: settings.autosave ?? DEFAULTS.autosave,
  };
}

function applyPatch(settings: Settings, patch: SettingsPatch): Settings {
  const defined = Object.fromEntries(Object.entries(patch).filter(([, value]) => value !== undefined));
  return { ...settings, ...defined };
}

function toPublic(settings: Resolved<Settings>): PublicSettings {
  const { apiToken, ...rest } = settings;
  return rest;
}
