type Listener<T> = (payload: T) => void;

class EventBus<Events extends Record<string, unknown>> {
  private listeners: { [K in keyof Events]?: Listener<Events[K]>[] } = {};

  on<K extends keyof Events>(event: K, listener: Listener<Events[K]>): void {
    (this.listeners[event] ??= []).push(listener);
  }

  emit<K extends keyof Events>(event: K, payload: Events[K]): void {
    for (const listener of this.listeners[event] ?? []) listener(payload);
  }
}

type CartItem = { sku: string; qty: number };
type ShopEvents = { added: CartItem; cleared: undefined };

class CartBadge {
  count = 0;

  constructor(bus: EventBus<ShopEvents>) {
    bus.on("added", this.onAdded);
    bus.on("cleared", () => this.reset());
  }

  onAdded(item: CartItem): void {
    this.count += item.qty;
  }

  reset(): void {
    this.count = 0;
  }
}
