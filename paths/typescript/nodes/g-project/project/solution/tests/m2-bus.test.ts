import { test } from "node:test";
import assert from "node:assert/strict";
import { EventBus } from "../src/bus.ts";

type ChatEvents = {
  message: { from: string; text: string };
  joined: string;
  typing: boolean;
};

test("listeners get the payload of their own event only", () => {
  const bus = new EventBus<ChatEvents>();
  const got: string[] = [];
  bus.on("message", (m) => got.push(`${m.from}: ${m.text}`));
  bus.on("joined", (name) => got.push(`+${name}`));
  bus.emit("joined", "ada");
  bus.emit("message", { from: "ada", text: "hi" });
  bus.emit("typing", true); // nobody listens: no error
  assert.deepEqual(got, ["+ada", "ada: hi"]);
});

test("several listeners run in subscription order", () => {
  const bus = new EventBus<ChatEvents>();
  const order: number[] = [];
  bus.on("typing", () => order.push(1));
  bus.on("typing", () => order.push(2));
  bus.emit("typing", true);
  assert.deepEqual(order, [1, 2]);
});

test("off and the returned unsubscribe both stop a listener", () => {
  const bus = new EventBus<ChatEvents>();
  let a = 0;
  let b = 0;
  const listenerA = () => a++;
  bus.on("typing", listenerA);
  const unsubscribeB = bus.on("typing", () => b++);
  bus.emit("typing", true);
  bus.off("typing", listenerA);
  unsubscribeB();
  bus.emit("typing", false);
  assert.deepEqual([a, b], [1, 1]);
  assert.equal(bus.listenerCount("typing"), 0);
});

test("once fires a single time", () => {
  const bus = new EventBus<ChatEvents>();
  const seen: string[] = [];
  bus.once("joined", (n) => seen.push(n));
  bus.emit("joined", "ada");
  bus.emit("joined", "grace");
  assert.deepEqual(seen, ["ada"]);
});

test("a listener unsubscribing during emit doesn't skip the next one", () => {
  const bus = new EventBus<ChatEvents>();
  const seen: string[] = [];
  const off = bus.on("typing", () => {
    seen.push("first");
    off();
  });
  bus.on("typing", () => seen.push("second"));
  bus.emit("typing", true);
  assert.deepEqual(seen, ["first", "second"]);
});

function typeChecks(bus: EventBus<ChatEvents>) {
  // @ts-expect-error not an event in the map
  bus.emit("left", "ada");
  // @ts-expect-error "joined" carries a string, not an object
  bus.emit("joined", { name: "ada" });
  // @ts-expect-error the message payload has no `body`
  bus.on("message", (m) => m.body);
  // @ts-expect-error the payload is required
  bus.emit("message");
}
void typeChecks;
