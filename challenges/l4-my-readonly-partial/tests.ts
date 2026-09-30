interface Todo {
  title: string;
  done: boolean;
  tags: string[];
}

type cases = [
  Expect<Equal<MyReadonly<Todo>, { readonly title: string; readonly done: boolean; readonly tags: string[] }>>,
  Expect<Equal<MyPartial<Todo>, { title?: string; done?: boolean; tags?: string[] }>>,
  Expect<Equal<MyReadonly<Todo>, Readonly<Todo>>>,
  Expect<Equal<MyPartial<Todo>, Partial<Todo>>>,
  // they compose
  Expect<Equal<MyReadonly<MyPartial<Todo>>, { readonly title?: string; readonly done?: boolean; readonly tags?: string[] }>>,
  // nothing in, nothing out
  Expect<Equal<MyPartial<{}>, {}>>,
];

declare const frozen: MyReadonly<Todo>;
// @ts-expect-error — readonly properties can't be reassigned
frozen.title = "changed";

const draft: MyPartial<Todo> = {};
const draft2: MyPartial<Todo> = { done: true };
// @ts-expect-error — optional, but still type-checked
const draft3: MyPartial<Todo> = { done: "yes" };
