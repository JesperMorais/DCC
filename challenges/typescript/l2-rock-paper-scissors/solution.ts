type Move = "rock" | "paper" | "scissors";
type Result = "player1" | "player2" | "draw";

function playRound(p1: Move, p2: Move): Result {
  if (p1 === p2) {
    return "draw";
  }
  if (
    (p1 === "rock" && p2 === "scissors") ||
    (p1 === "scissors" && p2 === "paper") ||
    (p1 === "paper" && p2 === "rock")
  ) {
    return "player1";
  }
  return "player2";
}
