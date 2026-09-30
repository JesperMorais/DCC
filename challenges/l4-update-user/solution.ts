interface User {
  id: number;
  name: string;
  email: string;
  role: "admin" | "member";
  updatedAt: number;
}

type UserPatch = Partial<Omit<User, "id" | "updatedAt">>;

function updateUser(user: User, patch: UserPatch, now: number): User {
  const next: User = { ...user, ...patch, updatedAt: now };
  if (patch.email !== undefined) {
    next.email = patch.email.trim().toLowerCase();
  }
  return next;
}
