type cases = [Expect<Equal<typeof assertNever, (value: never) => never>>];

// Never called: these lines are only here for the compiler (running them would throw).
const typeOnlyChecks = () => {
  // @ts-expect-error — only never can be passed to assertNever
  assertNever("in_transit");

  // @ts-expect-error — an in_transit status needs a location
  trackingLine({ kind: "in_transit", carrier: "DHL" });

  // @ts-expect-error — "lost" is not a ShipmentStatus kind
  isFinal({ kind: "lost" });
};

// Bad data from the API: types are erased, so this compiles but has a kind nobody handles.
const fromApi = JSON.parse('{ "kind": "teleported" }') as ShipmentStatus;

test("lines for the waiting and moving states", () => {
  expect(trackingLine({ kind: "label_created" })).toBe("Label created, waiting for pickup");
  expect(trackingLine({ kind: "in_transit", carrier: "PostNord", location: "Malmö" })).toBe(
    "In transit with PostNord, last seen in Malmö",
  );
  expect(trackingLine({ kind: "out_for_delivery", eta: "14:00" })).toBe("Out for delivery, expected by 14:00");
});

test("delivered with and without a signature", () => {
  expect(trackingLine({ kind: "delivered", signedBy: "Ada" })).toBe("Delivered, signed by Ada");
  expect(trackingLine({ kind: "delivered", signedBy: null })).toBe("Delivered");
});

test("returned shows the reason", () => {
  expect(trackingLine({ kind: "returned", reason: "Address not found" })).toBe(
    "Returned to sender: Address not found",
  );
});

test("isFinal is true only for delivered and returned", () => {
  expect(isFinal({ kind: "delivered", signedBy: null })).toBe(true);
  expect(isFinal({ kind: "returned", reason: "Refused" })).toBe(true);
  expect(isFinal({ kind: "label_created" })).toBe(false);
  expect(isFinal({ kind: "in_transit", carrier: "DHL", location: "Oslo" })).toBe(false);
  expect(isFinal({ kind: "out_for_delivery", eta: "09:30" })).toBe(false);
});

test("assertNever throws with the value in the message", () => {
  expect(() => assertNever({ kind: "x" } as never)).toThrow('Unexpected value: {"kind":"x"}');
});

test("an unknown kind from the API throws in both functions", () => {
  expect(() => trackingLine(fromApi)).toThrow('Unexpected value: {"kind":"teleported"}');
  expect(() => isFinal(fromApi)).toThrow('Unexpected value: {"kind":"teleported"}');
});
