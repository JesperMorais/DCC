function freeShipping(subtotal: number, weightKg: number, isMember: boolean): boolean {
  return (isMember || subtotal >= 50) && weightKg <= 20;
}

function shippingCost(subtotal: number, weightKg: number, isMember: boolean): number {
  if (freeShipping(subtotal, weightKg, isMember)) return 0;
  if (weightKg > 20) return 25;
  if (weightKg <= 1) return 4;
  if (weightKg <= 5) return 8;
  return 12;
}
