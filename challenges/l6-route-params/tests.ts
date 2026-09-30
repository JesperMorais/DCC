type cases = [
  Expect<Equal<Params<"/users/:id">, { id: string }>>,
  Expect<Equal<Params<"/users/:id/posts/:postId">, { id: string; postId: string }>>,
  Expect<Equal<Params<"/:a/:b/:c">, { a: string; b: string; c: string }>>,
  Expect<Equal<Params<"/shop/:category/items/:itemId/reviews">, { category: string; itemId: string }>>,
  Expect<Equal<Params<"/:slug">, { slug: string }>>,
  Expect<Equal<Params<"/about">, {}>>,
  Expect<Equal<Params<"/">, {}>>,
  Expect<Equal<Params<"">, {}>>,
];

// @ts-expect-error — routes are strings
type notARoute = Params<42>;

declare function route<P extends string>(path: P, params: Params<P>): string;

function _usage() {
  route("/users/:id/posts/:postId", { id: "1", postId: "9" });
  route("/about", {});
  // @ts-expect-error — postId is missing
  route("/users/:id/posts/:postId", { id: "1" });
  // @ts-expect-error — typo in the param name
  route("/users/:id", { ID: "1" });
}
