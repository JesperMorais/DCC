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
  return { theme: "light", fontSize: 0, emailAlerts: false };
}
