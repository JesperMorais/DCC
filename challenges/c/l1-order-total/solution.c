int order_total(int quantity, int unit_price, int shipping) {
    int items_cost = quantity * unit_price;
    return items_cost + shipping;
}
