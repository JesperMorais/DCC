type ParamNames<S extends string> = S extends `${string}:${infer P}/${infer Rest}`
  ? P | ParamNames<Rest>
  : S extends `${string}:${infer P}`
    ? P
    : never;

type Params<Path extends string> = { [K in ParamNames<Path>]: string };
