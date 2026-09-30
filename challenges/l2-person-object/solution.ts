type Person = {
  name: string;
  age: number;
};

function describe(person: Person): string {
  return `${person.name} is ${person.age} years old`;
}

function haveBirthday(person: Person): Person {
  return { name: person.name, age: person.age + 1 };
}
