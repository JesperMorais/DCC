function upNext(queue: readonly string[], count: number): string[] {
  return queue.slice(0, count);
}

function playNext(queue: readonly string[], song: string): string[] {
  const i = queue.indexOf(song);
  if (i === -1) return [song, ...queue];
  return [song, ...queue.slice(0, i), ...queue.slice(i + 1)];
}

function moveDown(queue: readonly string[], index: number): string[] {
  if (index < 0 || index >= queue.length - 1) return [...queue];
  return [...queue.slice(0, index), queue[index + 1], queue[index], ...queue.slice(index + 2)];
}
