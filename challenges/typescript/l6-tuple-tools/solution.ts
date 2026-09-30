type Length<T extends readonly unknown[]> = T["length"];

type TupleToUnion<T extends readonly unknown[]> = T[number];

type Last<T extends readonly unknown[]> = T extends readonly [...unknown[], infer L] ? L : never;
