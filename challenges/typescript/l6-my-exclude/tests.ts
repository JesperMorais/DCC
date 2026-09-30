type exclude_cases = [
  Expect<Equal<MyExclude<"a" | "b" | "c", "a">, "b" | "c">>,
  Expect<Equal<MyExclude<"a" | "b" | "c", "a" | "b">, "c">>,
  Expect<Equal<MyExclude<string | number | (() => void), Function>, string | number>>,
  Expect<Equal<MyExclude<string | number | boolean, number | boolean>, string>>,
  Expect<Equal<MyExclude<"a" | 1 | "b" | 2, string>, 1 | 2>>,
  Expect<Equal<MyExclude<"a", "a">, never>>,
  Expect<Equal<MyExclude<never, "a">, never>>,
  Expect<Equal<MyExclude<"x" | "y", never>, "x" | "y">>,
];

type isnever_cases = [
  Expect<Equal<IsNever<never>, true>>,
  Expect<Equal<IsNever<never | string>, false>>,
  Expect<Equal<IsNever<"">, false>>,
  Expect<Equal<IsNever<undefined>, false>>,
  Expect<Equal<IsNever<null>, false>>,
  Expect<Equal<IsNever<[]>, false>>,
  Expect<Equal<IsNever<{}>, false>>,
  Expect<Equal<IsNever<MyExclude<"a", "a">>, true>>,
];
