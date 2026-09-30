interface User {
  id: number;
  name: string;
  email: string;
  role: "admin" | "member";
  updatedAt: number;
}

// TODO: every field optional, but no `id` or `updatedAt`
type UserPatch = Partial<User>;

function updateUser(user: User, patch: UserPatch, now: number): User {
  return user;
}
