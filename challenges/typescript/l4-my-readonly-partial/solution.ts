type MyReadonly<T> = { readonly [K in keyof T]: T[K] };

type MyPartial<T> = { [K in keyof T]?: T[K] };
