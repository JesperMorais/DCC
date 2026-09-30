interface User {
  id: number;
  name: string;
  email?: string;
  roles: string[];
}

function isUser(value: unknown): value is User {
  if (typeof value !== "object" || value === null) return false;
  const v = value as Record<string, unknown>;
  return (
    typeof v.id === "number" &&
    typeof v.name === "string" &&
    (v.email === undefined || typeof v.email === "string") &&
    Array.isArray(v.roles) &&
    v.roles.every((r) => typeof r === "string")
  );
}

function assertUser(value: unknown): asserts value is User {
  if (!isUser(value)) throw new Error("Invalid user");
}
