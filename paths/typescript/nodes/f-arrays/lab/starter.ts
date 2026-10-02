function upNext(queue: any, count: any): any {
  return queue;
}

function playNext(queue: any, song: any): any {
  queue.unshift(song);
  return queue;
}

function moveDown(queue: any, index: any): any {
  return queue;
}
