type ShipmentStatus =
  | { kind: "label_created" }
  | { kind: "in_transit"; carrier: string; location: string }
  | { kind: "out_for_delivery"; eta: string }
  | { kind: "delivered"; signedBy: string | null }
  | { kind: "returned"; reason: string };

function assertNever(value: never): never {
  throw new Error(`Unexpected value: ${JSON.stringify(value)}`);
}

function trackingLine(status: ShipmentStatus): string {
  switch (status.kind) {
    case "label_created":
      return "Label created, waiting for pickup";
    case "in_transit":
      return `In transit with ${status.carrier}, last seen in ${status.location}`;
    case "out_for_delivery":
      return `Out for delivery, expected by ${status.eta}`;
    case "delivered":
      return status.signedBy === null ? "Delivered" : `Delivered, signed by ${status.signedBy}`;
    case "returned":
      return `Returned to sender: ${status.reason}`;
    default:
      return assertNever(status);
  }
}

function isFinal(status: ShipmentStatus): boolean {
  switch (status.kind) {
    case "delivered":
    case "returned":
      return true;
    case "label_created":
    case "in_transit":
    case "out_for_delivery":
      return false;
    default:
      return assertNever(status);
  }
}
