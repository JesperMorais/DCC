int cinema_ticket_price(int age) {
    if (age < 12) {
        return 60;
    } else if (age >= 65) {
        return 80;
    } else {
        return 120;
    }
}
