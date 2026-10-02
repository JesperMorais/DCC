function maskCard(card: string): string {
  return card.slice(-4).padStart(card.length, "*");
}

function slugify(title: string): string {
  return title.trim().toLowerCase().replaceAll(" ", "-");
}

function isPdf(filename: string): boolean {
  return filename.toLowerCase().endsWith(".pdf");
}
