function ticketPrice(age: number): number {
  if (age < 12) {
    return 5;
  } else if (age >= 65) {
    return 7;
  } else {
    return 10;
  }
}
