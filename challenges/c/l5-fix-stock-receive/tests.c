#include <stdbool.h>

TEST(a_new_sku_is_added) {
    Warehouse w = {0};
    EXPECT_TRUE(stock_receive(&w, "BOLT-M6", 100));
    EXPECT_EQ(stock_quantity(&w, "BOLT-M6"), 100);
    EXPECT_EQ(stock_quantity(&w, "NUT-M6"), 0);
}

TEST(receiving_a_known_sku_adds_to_it) {
    Warehouse w = {0};
    stock_receive(&w, "BOLT-M6", 100);
    EXPECT_TRUE(stock_receive(&w, "BOLT-M6", 40));
    EXPECT_EQ(stock_quantity(&w, "BOLT-M6"), 140);
    EXPECT_EQ(w.count, 1);
}

TEST(shipping_takes_units_out) {
    Warehouse w = {0};
    stock_receive(&w, "BOLT-M6", 100);
    EXPECT_TRUE(stock_ship(&w, "BOLT-M6", 30));
    EXPECT_EQ(stock_quantity(&w, "BOLT-M6"), 70);
}

TEST(cannot_ship_more_than_in_stock_or_an_unknown_sku) {
    Warehouse w = {0};
    stock_receive(&w, "BOLT-M6", 100);
    EXPECT_FALSE(stock_ship(&w, "BOLT-M6", 500));
    EXPECT_FALSE(stock_ship(&w, "WASHER-8", 1));
    EXPECT_EQ(stock_quantity(&w, "BOLT-M6"), 100);
}

TEST(receive_ship_receive_keeps_a_running_total) {
    Warehouse w = {0};
    stock_receive(&w, "NUT-M6", 50);
    stock_receive(&w, "BOLT-M6", 10);
    stock_ship(&w, "NUT-M6", 20);
    stock_receive(&w, "NUT-M6", 5);
    EXPECT_EQ(stock_quantity(&w, "NUT-M6"), 35);
    EXPECT_EQ(stock_quantity(&w, "BOLT-M6"), 10);
}

TEST(a_full_warehouse_rejects_new_skus_but_still_restocks) {
    Warehouse w = {0};
    const char *skus[MAX_ITEMS] = {"A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8"};
    for (int i = 0; i < MAX_ITEMS; i++) EXPECT_TRUE(stock_receive(&w, skus[i], 1));
    EXPECT_FALSE(stock_receive(&w, "A9", 1));
    EXPECT_EQ(stock_quantity(&w, "A9"), 0);
    EXPECT_TRUE(stock_receive(&w, "A8", 4));
    EXPECT_EQ(stock_quantity(&w, "A8"), 5);
}

TEST(rejects_a_sku_too_long_to_store) {
    Warehouse w = {0};
    EXPECT_FALSE(stock_receive(&w, "SKU-THAT-IS-WAY-TOO-LONG", 1));
    EXPECT_EQ(w.count, 0);
}
