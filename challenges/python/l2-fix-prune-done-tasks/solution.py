def clear_finished(tasks: list[str]) -> None:
    for task in list(tasks):
        if task.startswith("[x] "):
            tasks.remove(task)
