export type Listener<P> = (payload: P) => void;

/** For each event name, the listeners waiting for that event's payload. */
type ListenerMap<E> = { [K in keyof E]?: Listener<E[K]>[] };

/** A pub/sub hub whose event names and payloads come from one map type, `E`. */
export class EventBus<E extends object> {
  #listeners: ListenerMap<E> = {};

  on<K extends keyof E>(event: K, listener: Listener<E[K]>): () => void {
    this.#listeners[event] = [...(this.#listeners[event] ?? []), listener];
    return () => this.off(event, listener);
  }

  once<K extends keyof E>(event: K, listener: Listener<E[K]>): () => void {
    const off = this.on(event, (payload) => {
      off();
      listener(payload);
    });
    return off;
  }

  off<K extends keyof E>(event: K, listener: Listener<E[K]>): void {
    this.#listeners[event] = this.#listeners[event]?.filter((l) => l !== listener);
  }

  emit<K extends keyof E>(event: K, payload: E[K]): void {
    // The array is replaced, never mutated, on on/off, so a listener that
    // unsubscribes during emit doesn't make us skip the next one.
    for (const listener of this.#listeners[event] ?? []) listener(payload);
  }

  listenerCount(event: keyof E): number {
    return this.#listeners[event]?.length ?? 0;
  }
}
