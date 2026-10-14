interface Product {
  name: string;
  priceCents: number;
}

function cheapestFirst(products: Product[]): Product[] {
  return [...products].sort((a, b) => a.priceCents - b.priceCents);
}
