interface UserSettings {
  theme?: "light" | "dark";
  fontSize?: number;
  emailAlerts?: boolean;
}

interface ResolvedSettings {
  theme: "light" | "dark";
  fontSize: number;
  emailAlerts: boolean;
}

function resolveSettings(saved: UserSettings): ResolvedSettings {
  return {
    theme: saved.theme ?? "light",
    fontSize: saved.fontSize ?? 14,
    emailAlerts: saved.emailAlerts ?? true,
  };
}
