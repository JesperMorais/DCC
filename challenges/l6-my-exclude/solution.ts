type MyExclude<T, U> = T extends U ? never : T;

type IsNever<T> = [T] extends [never] ? true : false;
