import readline from "node:readline";
import { cartReducer, emptyCart, parseCommand, render } from "./cart.ts";
import { Store } from "./store.ts";

const store = new Store(emptyCart, cartReducer);

// The CLI never prints the cart itself after a command: it reacts to store events.
// Listeners run in subscription order, so the coupon message comes before the cart.
store.watch("coupon", (code) => {
  if (code) console.log(`coupon ${code} applied`);
});
store.on("change", ({ next }) => console.log(render(next)));
store.on("rejected", ({ error }) => console.log(`error: ${error.message}`));

const rl = readline.createInterface({ input: process.stdin, output: process.stdout, terminal: process.stdin.isTTY });
const prompt = () => {
  if (process.stdin.isTTY) rl.prompt();
};

console.log("typed-store cart. Commands: add <item> [qty], remove <item>, set <item> <qty>, coupon <code>, clear, undo, show, quit");
prompt();

rl.on("line", (line) => {
  const cmd = parseCommand(line);
  switch (cmd.kind) {
    case "action":
      store.dispatch(cmd.action);
      break;
    case "undo":
      if (!store.undo()) console.log("nothing to undo");
      break;
    case "show":
      console.log(render(store.state));
      break;
    case "quit":
      rl.close();
      return;
    case "error":
      console.log(`error: ${cmd.message}`);
      break;
  }
  prompt();
});
