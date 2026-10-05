// The data types for the expense tracker. These are given; the functions that
// use them are yours to design. Add more types if you want, but keep these.

/** One good row from the bank export. */
export interface Transaction {
  /** 1-based line number in the CSV file (the header is line 1). */
  line: number;
  /** "YYYY-MM-DD" */
  date: string;
  /** Trimmed, never empty. */
  description: string;
  /** Whole cents. Negative = money out (an expense), positive = money in. */
  cents: number;
}

/** A row that couldn't be read. It's skipped, with a warning. */
export interface BadRow {
  line: number;
  /** Exactly the text after "line N: " in the warning, e.g. `bad amount "abc"`. */
  reason: string;
}

/** What reading one CSV line gives you. Tell the two apart by their shape. */
export type Row = Transaction | BadRow;

/** One line of the rules file: "groceries: ica, coop, willys". */
export interface Rule {
  category: string;
  /** Lower-case, trimmed, never empty. */
  keywords: string[];
}

export type Command = "list" | "report" | "top" | "months";

/** The optional flags. A flag that wasn't given is simply missing. */
export interface Flags {
  rules?: string;
  month?: string;
}
