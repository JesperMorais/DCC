interface Product {
  sku: string;
  name: string;
  stock: number;
}

function lowStockReport(products: readonly Product[], threshold: number): string[] {
  return products
    .filter((p) => p.stock < threshold)
    .sort((a, b) => a.stock - b.stock || a.name.localeCompare(b.name))
    .map((p) => {
      const status = p.stock === 0 ? "out of stock" : `${p.stock} left`;
      return `${p.sku} ${p.name} (${status})`;
    });
}
