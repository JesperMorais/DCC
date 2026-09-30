int microwave_display(int seconds) {
    int minutes = seconds / 60;
    int rest = seconds % 60;
    return minutes * 100 + rest;
}
