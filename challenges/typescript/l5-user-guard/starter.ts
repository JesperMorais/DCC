interface User {
  id: number;
  name: string;
  email?: string;
  roles: string[];
}

function isUser(value: unknown): value is User {
  // TODO
  return false;
}

function assertUser(value: unknown): asserts value is User {
  // TODO
}
