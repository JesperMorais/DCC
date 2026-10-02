type Student = {
  name: string;
  scores: number[];
};

type Grade = "A" | "B" | "C" | "D" | "F";

type ReportCard = {
  name: string;
  average: number;
  grade: Grade;
};

function averageScore(scores: number[]): number {
  if (scores.length === 0) return 0;
  let total = 0;
  for (const score of scores) total += score;
  return Math.round((total / scores.length) * 10) / 10;
}

function gradeFor(average: number): Grade {
  if (average >= 90) return "A";
  if (average >= 75) return "B";
  if (average >= 60) return "C";
  if (average >= 50) return "D";
  return "F";
}

function reportCard(student: Student): ReportCard {
  const average = averageScore(student.scores);
  return { name: student.name.trim(), average, grade: gradeFor(average) };
}

function honorRoll(students: Student[]): string[] {
  return students
    .map(reportCard)
    .filter((card) => card.grade === "A")
    .map((card) => card.name);
}

function curve(student: Student, points: number): Student {
  return { name: student.name, scores: student.scores.map((s) => Math.min(100, s + points)) };
}
