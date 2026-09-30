type Config = {
  name: string;
  retries: number;
  db: { host: string; ports: number[]; auth: { user: string; pass: string | null } };
  onError: (e: Error) => void;
  pair: [1, { tag: "a" }];
};

type FrozenConfig = {
  readonly name: string;
  readonly retries: number;
  readonly db: {
    readonly host: string;
    readonly ports: readonly number[];
    readonly auth: { readonly user: string; readonly pass: string | null };
  };
  readonly onError: (e: Error) => void;
  readonly pair: readonly [1, { readonly tag: "a" }];
};

type cases = [
  Expect<Equal<DeepReadonly<Config>, FrozenConfig>>,
  Expect<Equal<DeepReadonly<{ a: string[][] }>, { readonly a: readonly (readonly string[])[] }>>,
  Expect<Equal<DeepReadonly<string>, string>>,
  Expect<Equal<DeepReadonly<() => 1>, () => 1>>,
  Expect<Equal<DeepReadonly<{}>, {}>>,
  ExpectFalse<Equal<DeepReadonly<Config>, Readonly<Config>>>,
];

function _mutationsAreErrors(cfg: DeepReadonly<Config>) {
  // @ts-expect-error
  cfg.name = "x";
  // @ts-expect-error — nested objects too
  cfg.db.auth.user = "root";
  // @ts-expect-error — arrays lose their mutating methods
  cfg.db.ports.push(80);
  // functions must still be callable:
  cfg.onError(new Error("boom"));
}
