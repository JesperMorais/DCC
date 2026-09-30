interface Person {
  name: string;
  age: number;
  isAdmin: boolean;
}

declare const secret: unique symbol;

interface Mixed {
  id: string;
  0: "zero";
  [secret]: number;
  tags: string[];
}

type cases = [
  Expect<Equal<Getters<Person>, { getName: () => string; getAge: () => number; getIsAdmin: () => boolean }>>,
  Expect<Equal<Getters<Mixed>, { getId: () => string; getTags: () => string[] }>>,
  Expect<Equal<Getters<{ x: 1 | 2 }>, { getX: () => 1 | 2 }>>,
  Expect<Equal<Getters<{ fn: (n: number) => void }>, { getFn: () => (n: number) => void }>>,
  Expect<Equal<Getters<{}>, {}>>,
];

function _useGetters(p: Getters<Person>) {
  const n: string = p.getName();
  // @ts-expect-error — the original key is gone
  p.name;
  // @ts-expect-error — getters take no arguments
  p.getAge(1);
}
