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
  switch (action.type) {
    case "add": {
      const { line } = action;
      if (state.lines.some((l) => l.id === line.id)) {
        return { lines: state.lines.map((l) => (l.id === line.id ? { ...l, qty: l.qty + 1 } : l)) };
      }
      return { lines: [...state.lines, { ...line, qty: 1 }] };
    }
    case "remove":
      return { lines: state.lines.filter((l) => l.id !== action.id) };
    case "setQty": {
      const { id, qty } = action;
      if (qty <= 0) return { lines: state.lines.filter((l) => l.id !== id) };
      return { lines: state.lines.map((l) => (l.id === id ? { ...l, qty } : l)) };
    }
    case "clear":
      return { lines: [] };
    default: {
      const unhandled: never = action;
      throw new Error(`Unknown action: ${JSON.stringify(unhandled)}`);
    }
  }
}
