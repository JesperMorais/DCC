type gradeBookCases = [
  Expect<Equal<Grade, "A" | "B" | "C" | "D" | "F">>,
  Expect<Equal<ReportCard, { name: string; average: number; grade: Grade }>>,
];

// @ts-expect-error — there is no grade "E"
const missingGrade: Grade = "E";

const classOf2026: Student[] = [
  { name: "  Ada ", scores: [90, 93] },
  { name: "Linus", scores: [70, 75, 77] },
  { name: "Grace ", scores: [100, 95, 99] },
  { name: "Alan", scores: [] },
];

test("averageScore rounds to one decimal", () => {
  expect(averageScore([90, 93])).toBe(91.5);
  expect(averageScore([61, 62, 62])).toBe(61.7);
  expect(averageScore([70, 75, 77])).toBe(74);
});

test("averageScore of no scores is 0", () => {
  expect(averageScore([])).toBe(0);
});

test("gradeFor uses the bands, boundaries included", () => {
  expect(gradeFor(90)).toBe("A");
  expect(gradeFor(89.9)).toBe("B");
  expect(gradeFor(75)).toBe("B");
  expect(gradeFor(74.9)).toBe("C");
  expect(gradeFor(60)).toBe("C");
  expect(gradeFor(50)).toBe("D");
  expect(gradeFor(49.9)).toBe("F");
});

test("reportCard trims the name and grades the rounded average", () => {
  expect(reportCard({ name: "  Ada ", scores: [90, 93] })).toEqual({ name: "Ada", average: 91.5, grade: "A" });
  expect(reportCard({ name: "Kim", scores: [89.94, 90] })).toEqual({ name: "Kim", average: 90, grade: "A" });
  expect(reportCard({ name: "Alan", scores: [] })).toEqual({ name: "Alan", average: 0, grade: "F" });
});

test("honorRoll lists the A students in order", () => {
  expect(honorRoll(classOf2026)).toEqual(["Ada", "Grace"]);
  expect(honorRoll([])).toEqual([]);
});

test("curve adds points and caps at 100", () => {
  expect(curve({ name: "Linus", scores: [70, 98] }, 5)).toEqual({ name: "Linus", scores: [75, 100] });
  expect(curve({ name: " Mo", scores: [] }, 10)).toEqual({ name: " Mo", scores: [] });
});

test("curve does not change the original student", () => {
  const original: Student = { name: "Grace", scores: [80, 99] };
  const curved = curve(original, 3);
  expect(original.scores).toEqual([80, 99]);
  expect(curved.scores).toEqual([83, 100]);
  expect(curved.scores).not.toBe(original.scores);
});
