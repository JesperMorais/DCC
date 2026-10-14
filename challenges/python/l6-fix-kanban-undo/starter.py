type Columns = dict[str, list[str]]


class KanbanBoard:
    def __init__(self, columns: list[str], max_undo: int = 50) -> None:
        if not columns:
            raise ValueError("a board needs at least one column")
        self._columns: Columns = {name: [] for name in columns}
        self._history: list[Columns] = []
        self._max_undo = max_undo

    def _checkpoint(self) -> None:
        self._history.append(self._columns.copy())
        if len(self._history) > self._max_undo:
            del self._history[0]

    def _column(self, name: str) -> list[str]:
        try:
            return self._columns[name]
        except KeyError:
            raise KeyError(f"no such column: {name}") from None

    def _find(self, card: str) -> str:
        for name, cards in self._columns.items():
            if card in cards:
                return name
        raise KeyError(f"no such card: {card}")

    def add_column(self, name: str) -> None:
        if name in self._columns:
            raise ValueError(f"column exists: {name}")
        self._checkpoint()
        self._columns[name] = []

    def add_card(self, card: str, column: str | None = None) -> None:
        target = self._column(column if column is not None else next(iter(self._columns)))
        if any(card in cards for cards in self._columns.values()):
            raise ValueError(f"duplicate card: {card}")
        self._checkpoint()
        target.append(card)

    def move_card(self, card: str, to: str, position: int | None = None) -> None:
        source = self._column(self._find(card))
        dest = self._column(to)
        self._checkpoint()
        source.remove(card)
        dest.insert(len(dest) if position is None else position, card)

    def undo(self) -> bool:
        if not self._history:
            return False
        self._columns = self._history.pop()
        return True

    def snapshot(self) -> Columns:
        return {name: list(cards) for name, cards in self._columns.items()}
