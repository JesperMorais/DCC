import { InvalidActionError } from "./store.ts";

/** Prices in cents, so totals stay exact. */
export const CATALOG = { apple: 50, bread: 225, coffee: 799, cheese: 440 } as const;
export type Sku = keyof typeof CATALOG;

/** Percent off the subtotal. */
export const COUPONS: Record<string, number> = { SAVE10: 10, HALF: 50 };

export interface CartLine {
  sku: Sku;
  qty: number;
}

export interface CartState {
  lines: readonly CartLine[];
  coupon: string | null;
}

export type CartAction =
  | { type: "add"; sku: Sku; qty: number }
  | { type: "remove"; sku: Sku }
  | { type: "setQty"; sku: Sku; qty: number }
  | { type: "applyCoupon"; code: string }
  | { type: "clear" };

export const emptyCart: CartState = { lines: [], coupon: null };

function assertNever(value: never): never {
  throw new Error(`Unexpected value: ${JSON.stringify(value)}`);
}

function checkQty(action: CartAction & { qty: number }): void {
  if (!Number.isInteger(action.qty) || action.qty < 1) throw new InvalidActionError(action, `Quantity must be a whole number of at least 1, got ${action.qty}`);
}

export function cartReducer(state: CartState, action: CartAction): CartState {
  switch (action.type) {
    case "add": {
      checkQty(action);
      const existing = state.lines.find((l) => l.sku === action.sku);
      const lines = existing
        ? state.lines.map((l) => (l === existing ? { ...l, qty: l.qty + action.qty } : l))
        : [...state.lines, { sku: action.sku, qty: action.qty }];
      return { ...state, lines };
    }
    case "remove":
      if (!state.lines.some((l) => l.sku === action.sku)) throw new InvalidActionError(action, `${action.sku} is not in the cart`);
      return { ...state, lines: state.lines.filter((l) => l.sku !== action.sku) };
    case "setQty":
      checkQty(action);
      if (!state.lines.some((l) => l.sku === action.sku)) throw new InvalidActionError(action, `${action.sku} is not in the cart`);
      return { ...state, lines: state.lines.map((l) => (l.sku === action.sku ? { ...l, qty: action.qty } : l)) };
    case "applyCoupon": {
      const code = action.code.toUpperCase();
      if (!(code in COUPONS)) throw new InvalidActionError(action, `Unknown coupon ${action.code}`);
      return { ...state, coupon: code };
    }
    case "clear":
      return state.lines.length || state.coupon ? emptyCart : state;
    default:
      return assertNever(action);
  }
}

export function total(state: Readonly<CartState>): number {
  const subtotal = state.lines.reduce((sum, l) => sum + CATALOG[l.sku] * l.qty, 0);
  const percent = state.coupon ? (COUPONS[state.coupon] ?? 0) : 0;
  return Math.round((subtotal * (100 - percent)) / 100);
}

export const money = (cents: number) => `$${(cents / 100).toFixed(2)}`;

export type Command =
  | { kind: "action"; action: CartAction }
  | { kind: "undo" }
  | { kind: "show" }
  | { kind: "quit" }
  | { kind: "error"; message: string };

const isSku = (word: string): word is Sku => Object.hasOwn(CATALOG, word);

/** Turns one input line ("add apple 3") into a command. Never throws. */
export function parseCommand(line: string): Command {
  const [verb = "", ...args] = line.trim().split(/\s+/);
  const sku = args[0] ?? "";
  const qty = Number(args[1] ?? "1");
  const needSku = (make: (sku: Sku) => CartAction): Command =>
    isSku(sku) ? { kind: "action", action: make(sku) } : { kind: "error", message: `Unknown product "${sku}". Try: ${Object.keys(CATALOG).join(", ")}` };

  switch (verb.toLowerCase()) {
    case "add":
      return needSku((s) => ({ type: "add", sku: s, qty }));
    case "remove":
      return needSku((s) => ({ type: "remove", sku: s }));
    case "set":
      return needSku((s) => ({ type: "setQty", sku: s, qty }));
    case "coupon":
      return { kind: "action", action: { type: "applyCoupon", code: sku } };
    case "clear":
      return { kind: "action", action: { type: "clear" } };
    case "undo":
      return { kind: "undo" };
    case "show":
    case "":
      return { kind: "show" };
    case "quit":
    case "exit":
      return { kind: "quit" };
    default:
      return { kind: "error", message: `Unknown command "${verb}". Try: add, remove, set, coupon, clear, undo, show, quit` };
  }
}

export function render(state: Readonly<CartState>): string {
  if (!state.lines.length) return "  (cart is empty)";
  const rows = state.lines.map((l) => `  ${String(l.qty).padStart(3)} x ${l.sku.padEnd(8)} ${money(CATALOG[l.sku] * l.qty).padStart(8)}`);
  const coupon = state.coupon ? [`  coupon ${state.coupon} (-${COUPONS[state.coupon]}%)`] : [];
  return [...rows, ...coupon, `  total${" ".repeat(10)}${money(total(state)).padStart(8)}`].join("\n");
}
