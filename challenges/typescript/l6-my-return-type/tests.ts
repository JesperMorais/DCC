const fn = (v: boolean) => (v ? 1 : 2);
const fn1 = (v: boolean, w: any) => (v ? 1 : 2);

type cases = [
  Expect<Equal<string, MyReturnType<() => string>>>,
  Expect<Equal<123, MyReturnType<() => 123>>>,
  Expect<Equal<1 | 2, MyReturnType<(a: number, b: string) => 1 | 2>>>,
  Expect<Equal<Promise<boolean>, MyReturnType<() => Promise<boolean>>>>,
  Expect<Equal<() => "foo", MyReturnType<() => () => "foo">>>,
  Expect<Equal<1 | 2, MyReturnType<typeof fn>>>,
  Expect<Equal<1 | 2, MyReturnType<typeof fn1>>>,
  Expect<Equal<number, MyReturnType<typeof Math.random>>>,
  Expect<Equal<never, MyReturnType<() => never>>>,
  Expect<Equal<void, MyReturnType<(...args: string[]) => void>>>,
];

// @ts-expect-error — not a function type
type notAFunction = MyReturnType<string>;
