def clear_finished(tasks: list[str]) -> None:
    for task in tasks:
        if task.startswith("[x] "):
            tasks.remove(task)
