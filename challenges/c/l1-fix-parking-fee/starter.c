int garage_parking_fee(int hours) {
    int free_hours = 2;
    int price_per_hour = 15;
    if (hours <= free_hours) {
        return 0;
    }
    return hours * price_per_hour;
}
