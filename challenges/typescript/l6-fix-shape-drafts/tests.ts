type kind_cases = [
  Expect<Equal<ShapeKind<Shape>, "circle" | "rect" | "text">>,
  Expect<Equal<OfKind<Shape, "circle">, { kind: "circle"; id: string; radius: number }>>,
];

type draft_cases = [
  Expect<Equal<Draft<OfKind<Shape, "rect">>, { kind: "rect"; width: number; height: number }>>,
  Expect<
    Equal<
      Draft<Shape>,
      | { kind: "circle"; radius: number }
      | { kind: "rect"; width: number; height: number }
      | { kind: "text"; content: string; fontSize?: number }
    >
  >,
  Expect<Equal<Without<{ a: 1; b: 2 } | { a: 3; c: 4 }, "a">, { b: 2 } | { c: 4 }>>,
];

type patch_cases = [
  Expect<Equal<Patch<OfKind<Shape, "circle">>, { kind: "circle"; radius?: number }>>,
  Expect<Equal<Saved<OfKind<Shape, "circle">>, { kind: "circle"; id: string; radius: number; savedAt: number }>>,
];

declare function createShape(draft: Draft<Shape>): Shape;

function _useDrafts() {
  createShape({ kind: "circle", radius: 4 });
  createShape({ kind: "text", content: "Hello" });
  // @ts-expect-error — a circle has no width
  createShape({ kind: "circle", radius: 4, width: 10 });
  // @ts-expect-error — a rect needs its height
  createShape({ kind: "rect", width: 10 });
}
