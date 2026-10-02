type Student = {
  name: string;
  scores: number[];
};

type Grade = string;

type ReportCard = {};

function averageScore(scores: number[]): number {
  return 0;
}

function gradeFor(average: number): Grade {
  return "";
}

function reportCard(student: Student): ReportCard {
  return {};
}

function honorRoll(students: Student[]): string[] {
  return [];
}

function curve(student: Student, points: number): Student {
  return student;
}
