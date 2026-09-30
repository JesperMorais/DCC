const planets = ["mercury", "venus", "earth"] as const;

type length_cases = [
  Expect<Equal<Length<["a", "b", "c"]>, 3>>,
  Expect<Equal<Length<[]>, 0>>,
  Expect<Equal<Length<[undefined]>, 1>>,
  Expect<Equal<Length<typeof planets>, 3>>,
  Expect<Equal<Length<string[]>, number>>,
];

type union_cases = [
  Expect<Equal<TupleToUnion<[1, "two", true]>, 1 | "two" | true>>,
  Expect<Equal<TupleToUnion<["a", "a", "b"]>, "a" | "b">>,
  Expect<Equal<TupleToUnion<typeof planets>, "mercury" | "venus" | "earth">>,
  Expect<Equal<TupleToUnion<[]>, never>>,
];

type last_cases = [
  Expect<Equal<Last<[string, number, boolean]>, boolean>>,
  Expect<Equal<Last<[42]>, 42>>,
  Expect<Equal<Last<[]>, never>>,
  Expect<Equal<Last<typeof planets>, "earth">>,
  Expect<Equal<Last<[() => 1, { a: 1 }]>, { a: 1 }>>,
];

// @ts-expect-error — strings are not tuples
type bad1 = Length<"hello">;
// @ts-expect-error
type bad2 = TupleToUnion<{ 0: "a"; length: 1 }>;
// @ts-expect-error
type bad3 = Last<number>;
