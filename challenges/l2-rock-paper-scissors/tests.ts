type moveCases = [Expect<Equal<Move, "rock" | "paper" | "scissors">>];

// @ts-expect-error — "lizard" must not be a valid Move
const notAMove: Move = "lizard";

test("rock beats scissors", () => {
  expect(playRound("rock", "scissors")).toBe("player1");
  expect(playRound("scissors", "rock")).toBe("player2");
});

test("scissors beats paper", () => {
  expect(playRound("scissors", "paper")).toBe("player1");
  expect(playRound("paper", "scissors")).toBe("player2");
});

test("paper beats rock", () => {
  expect(playRound("paper", "rock")).toBe("player1");
  expect(playRound("rock", "paper")).toBe("player2");
});

test("same move is a draw", () => {
  expect(playRound("paper", "paper")).toBe("draw");
  expect(playRound("rock", "rock")).toBe("draw");
  expect(playRound("scissors", "scissors")).toBe("draw");
});
