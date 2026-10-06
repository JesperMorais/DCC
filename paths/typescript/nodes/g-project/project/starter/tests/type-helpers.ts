// Exact type equality for type-level tests. `Expect<Equal<A, B>>` compiles only when A and B
// are the same type: not merely assignable, the same. Use them in a list of cases:
//
//   type _cases = [Expect<Equal<typeof volume, number>>];
//
// If the types differ, `npm run check` reports "Type 'false' does not satisfy the constraint 'true'".
export type Equal<X, Y> = (<T>() => T extends X ? 1 : 2) extends <T>() => T extends Y ? 1 : 2 ? true : false;
export type Expect<T extends true> = T;
