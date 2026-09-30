interface CartItem {
  name: string;
  priceCents: number;
  quantity: number;
}

function cartTotal(items: CartItem[]): number {
  return items.reduce((sum, item) => sum + item.priceCents * item.quantity, 0);
}

function formatCents(cents: number): string {
  return `$${(cents / 100).toFixed(2)}`;
}
