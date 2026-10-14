type Shape =
  | { kind: "circle"; id: string; radius: number }
  | { kind: "rect"; id: string; width: number; height: number }
  | { kind: "text"; id: string; content: string; fontSize?: number };

type Simplify<T> = { [K in keyof T]: T[K] } & {};

type ShapeKind<T extends { kind: string }> = T["kind"];

type OfKind<T extends { kind: string }, K extends ShapeKind<T>> = Extract<T, { kind: K }>;

type Without<T, K extends PropertyKey> = T extends unknown ? Simplify<Omit<T, K>> : never;

type Draft<T extends { id: string }> = Without<T, "id">;

type Patch<T extends { kind: string; id: string }> = T extends unknown
  ? Simplify<{ kind: T["kind"] } & Partial<Without<T, "kind" | "id">>>
  : never;

type Handlers<T extends { kind: string }, R> = {
  [K in ShapeKind<T>]: (shape: OfKind<T, K>) => R;
};

type Saved<T extends { id: string }> = Simplify<T & { savedAt: number }>;

function renderShape(shape: Shape, handlers: Handlers<Shape, string>): string {
  switch (shape.kind) {
    case "circle":
      return handlers.circle(shape);
    case "rect":
      return handlers.rect(shape);
    case "text":
      return handlers.text(shape);
  }
}

function applyPatch<S extends Shape>(shape: S, patch: Patch<S>): S {
  return { ...shape, ...patch };
}

const shapeLabels: Handlers<Shape, string> = {
  circle: (s) => `circle r=${s.radius}`,
  rect: (s) => `rect ${s.width}×${s.height}`,
  text: (s) => `text "${s.content}"`,
};
