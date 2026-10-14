function isOnGuestList(guests: string[], name: string): boolean {
  if (guests.indexOf(name)) {
    return true;
  }
  return false;
}
