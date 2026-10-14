**Bug report from a beta tester:** "Undo on the board is weird. If I add a column and press undo, the column disappears, perfect. But if I drag a card to *Doing* and press undo, nothing happens. Pressing it again doesn't help either, the card just stays where it is."

`KanbanBoard(columns, max_undo=50)` holds named columns of cards:

- `add_column(name)`, `add_card(card, column=None)` (default: the first column) and `move_card(card, to, position=None)` (default: the end) change the board. Invalid changes raise (`ValueError` for duplicates, `KeyError` for unknown cards or columns) and change nothing.
- `undo()` reverts the **most recent change** and returns `True`, or returns `False` if there's nothing to undo. Only the last `max_undo` changes can be undone.
- `snapshot()` returns the board as `{column: [cards]}`.

```python
board = KanbanBoard(["Todo", "Done"])
board.add_card("a")
board.add_column("Review")
board.undo()        # True
board.snapshot()    # {"Todo": ["a"], "Done": []}
```

**Reproduction:**

```python
board = KanbanBoard(["Todo", "Doing"])
board.add_card("fix login")
board.move_card("fix login", "Doing")
board.undo()        # True
board.snapshot()
# returns  {"Todo": [], "Doing": ["fix login"]}
# expected {"Todo": ["fix login"], "Doing": []}
```
