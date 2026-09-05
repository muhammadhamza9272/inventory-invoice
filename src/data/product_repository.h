#pragma once

#include "model/product.h"

#include <QSqlDatabase>
#include <QSqlQuery>

#include <optional>
#include <vector>

namespace inv {

// All product persistence goes through here. Callers work with Product
// values; this class maps them to/from SQLite rows.
class ProductRepository
{
public:
    explicit ProductRepository(QSqlDatabase db) : m_db(std::move(db)) {}

    bool create(const Product &product, int *outId = nullptr);
    bool update(const Product &product);
    bool remove(int id);

    std::optional<Product> findById(int id) const;
    std::vector<Product> list(const ProductFilter &filter = {}) const;
    bool addStock(int id, int delta); // + to receive, - to remove

    QString lastError() const { return m_lastError; }

private:
    Product rowToProduct(const QSqlQuery &q) const;

    QSqlDatabase m_db;
    mutable QString m_lastError;
};

} // namespace inv
