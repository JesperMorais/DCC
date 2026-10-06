import { test } from "node:test";
import assert from "node:assert/strict";
import { cartReducer, emptyCart, parseCommand, render, total, type CartState } from "../src/cart.ts";
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

test("render prints the lines, the coupon and an exact total", () => {
  const cart: CartState = { lines: [{ sku: "apple", qty: 3 }], coupon: "HALF" };
  const out = render(cart);
  assert.match(out, /3 x apple\s+\$1\.50/);
  assert.match(out, /coupon HALF \(-50%\)/);
  assert.match(out, /total\s+\$0\.75$/);
  assert.match(render(emptyCart), /empty/);
});
