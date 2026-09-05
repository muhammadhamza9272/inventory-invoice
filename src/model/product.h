#pragma once

#include <QString>

#include <optional>

namespace inv {

// A product held in inventory. Stock is tracked as a plain integer
// (whole units) which covers the vast majority of small-business use.
struct Product {
    int id{-1};                  // -1 == not persisted yet
    QString name;
    QString sku;                 // stock-keeping unit / barcode lookalike
    double unitPrice{0.0};       // sale price per unit (before tax)
    int stock{0};                // quantity on hand
    int reorderLevel{0};         // alert when stock drops to or below this
};

struct ProductFilter {
    QString query;               // matches name/sku (case-insensitive substring)
    bool lowStockOnly{false};    // only products at or below reorder level
};

} // namespace inv
