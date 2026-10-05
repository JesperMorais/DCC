import { test } from "node:test";
import assert from "node:assert/strict";
import { execFileSync } from "node:child_process";
import { cartReducer, emptyCart, parseCommand, total, type CartState } from "../src/cart.ts";
import { InvalidActionError } from "../src/store.ts";

test("adding the same item twice adds up the quantity", () => {
  let cart = cartReducer(emptyCart, { type: "add", sku: "apple", qty: 2 });
  cart = cartReducer(cart, { type: "add", sku: "apple", qty: 3 });
  assert.deepEqual(cart.lines, [{ sku: "apple", qty: 5 }]);
  assert.equal(total(cart), 250);
});

test("coupons take a percentage off the total", () => {
  const cart: CartState = { lines: [{ sku: "coffee", qty: 2 }], coupon: null };
  assert.equal(total(cart), 1598);
  assert.equal(total(cartReducer(cart, { type: "applyCoupon", code: "save10" })), 1438);
});

test("invalid cart actions throw InvalidActionError", () => {
  assert.throws(() => cartReducer(emptyCart, { type: "remove", sku: "bread" }), InvalidActionError);
  assert.throws(() => cartReducer(emptyCart, { type: "add", sku: "bread", qty: 0 }), InvalidActionError);
  assert.throws(() => cartReducer(emptyCart, { type: "applyCoupon", code: "FREE" }), InvalidActionError);
});

test("parseCommand understands the commands and rejects junk", () => {
  assert.deepEqual(parseCommand("add apple 3"), { kind: "action", action: { type: "add", sku: "apple", qty: 3 } });
  assert.deepEqual(parseCommand("  add bread"), { kind: "action", action: { type: "add", sku: "bread", qty: 1 } });
  assert.deepEqual(parseCommand("undo"), { kind: "undo" });
  assert.equal(parseCommand("add pear 1").kind, "error");
  assert.equal(parseCommand("dance").kind, "error");
});

test("the CLI reacts to store events", () => {
  const input = ["add apple 3", "add pear", "coupon HALF", "set apple 0", "undo", "quit"].join("\n");
  const out = execFileSync(process.execPath, ["--import", "tsx", "src/main.ts"], { input, encoding: "utf8" });
  assert.match(out, /3 x apple\s+\$1\.50/);
  assert.match(out, /error: Unknown product "pear"/);
  assert.match(out, /coupon HALF applied/);
  assert.match(out, /total\s+\$0\.75/);
  assert.match(out, /error: Quantity must be/);
  assert.equal(out.trim().split("\n").at(-1)?.trim().startsWith("total"), true); // undo reprints the cart
});
