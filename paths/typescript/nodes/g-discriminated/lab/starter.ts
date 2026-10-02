type ShipmentStatus =
  | { kind: "label_created" }
  | { kind: "in_transit"; carrier: string; location: string }
  | { kind: "out_for_delivery"; eta: string }
  | { kind: "delivered"; signedBy: string | null }
  | { kind: "returned"; reason: string };

function assertNever(value: any): any {
  return value;
}

function trackingLine(status: ShipmentStatus): string {
  return "";
}

function isFinal(status: ShipmentStatus): boolean {
  return false;
}
