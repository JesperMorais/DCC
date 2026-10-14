#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#define MAX_ITEMS 8
#define SKU_LEN 16

typedef struct {
    char sku[SKU_LEN];
    int quantity;
} StockItem;

typedef struct {
    StockItem items[MAX_ITEMS];
    size_t count;
} Warehouse;

static StockItem *find_item(Warehouse *w, const char *sku) {
    for (size_t i = 0; i < w->count; i++) {
        if (strcmp(w->items[i].sku, sku) == 0) {
            return &w->items[i];
        }
    }
    return NULL;
}

bool stock_receive(Warehouse *w, const char *sku, int quantity) {
    for (size_t i = 0; i < w->count; i++) {
        StockItem item = w->items[i];
        if (strcmp(item.sku, sku) == 0) {
            item.quantity += quantity;
            return true;
        }
    }
    if (w->count == MAX_ITEMS || strlen(sku) >= SKU_LEN) {
        return false;
    }
    StockItem *slot = &w->items[w->count++];
    strcpy(slot->sku, sku);
    slot->quantity = quantity;
    return true;
}

bool stock_ship(Warehouse *w, const char *sku, int quantity) {
    StockItem *item = find_item(w, sku);
    if (item == NULL || item->quantity < quantity) {
        return false;
    }
    item->quantity -= quantity;
    return true;
}

int stock_quantity(Warehouse *w, const char *sku) {
    const StockItem *item = find_item(w, sku);
    return item ? item->quantity : 0;
}
