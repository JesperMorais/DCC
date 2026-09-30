interface CartLine {
  id: string;
  name: string;
  priceCents: number;
  qty: number;
}

interface CartState {
  lines: CartLine[];
}

type CartAction =
  | { type: "add"; line: Omit<CartLine, "qty"> }
  | { type: "remove"; id: string }
  | { type: "setQty"; id: string; qty: number }
  | { type: "clear" };

function cartReducer(state: CartState, action: CartAction): CartState {
  return state;
}
