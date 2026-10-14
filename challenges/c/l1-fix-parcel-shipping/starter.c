int parcel_shipping_cost(int grams) {
    int cost = 49;
    if (grams > 5000) {
        cost = 129;
    }
    if (grams > 1000) {
        cost = 79;
    }
    return cost;
}
