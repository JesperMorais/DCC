function isOnGuestList(guests: string[], name: string): boolean {
  if (guests.indexOf(name) !== -1) {
    return true;
  }
  return false;
}
