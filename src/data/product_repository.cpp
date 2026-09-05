#include "product_repository.h"

#include <QSqlError>
#include <QSqlQuery>
#include <QStringList>
#include <QVariant>

namespace inv {

namespace {
QString nonNull(const QString &s) { return s.isEmpty() ? QStringLiteral("") : s; }
}

Product ProductRepository::rowToProduct(const QSqlQuery &q) const
{
    Product p;
    p.id = q.value(0).toInt();
    p.name = q.value(1).toString();
    p.sku = q.value(2).toString();
    p.unitPrice = q.value(3).toDouble();
    p.stock = q.value(4).toInt();
    p.reorderLevel = q.value(5).toInt();
    return p;
}

bool ProductRepository::create(const Product &product, int *outId)
{
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO products (name, sku, unit_price, stock, reorder_level) "
              "VALUES (?, ?, ?, ?, ?)");
    q.addBindValue(nonNull(product.name));
    q.addBindValue(nonNull(product.sku));
    q.addBindValue(product.unitPrice);
    q.addBindValue(product.stock);
    q.addBindValue(product.reorderLevel);
    if (!q.exec()) {
        m_lastError = q.lastError().text();
        return false;
    }
    if (outId)
        *outId = q.lastInsertId().toInt();
    return true;
}

bool ProductRepository::update(const Product &product)
{
    QSqlQuery q(m_db);
    q.prepare("UPDATE products SET name=?, sku=?, unit_price=?, stock=?, reorder_level=? "
              "WHERE id=?");
    q.addBindValue(nonNull(product.name));
    q.addBindValue(nonNull(product.sku));
    q.addBindValue(product.unitPrice);
    q.addBindValue(product.stock);
    q.addBindValue(product.reorderLevel);
    q.addBindValue(product.id);
    if (!q.exec()) {
        m_lastError = q.lastError().text();
        return false;
    }
    return true;
}

bool ProductRepository::remove(int id)
{
    QSqlQuery q(m_db);
    q.prepare("DELETE FROM products WHERE id=?");
    q.addBindValue(id);
    if (!q.exec()) {
        m_lastError = q.lastError().text();
        return false;
    }
    return true;
}

std::optional<Product> ProductRepository::findById(int id) const
{
    QSqlQuery q(m_db);
    q.prepare("SELECT id, name, sku, unit_price, stock, reorder_level "
              "FROM products WHERE id=?");
    q.addBindValue(id);
    if (!q.exec() || !q.next())
        return std::nullopt;
    return rowToProduct(q);
}

std::vector<Product> ProductRepository::list(const ProductFilter &filter) const
{
    QString sql = "SELECT id, name, sku, unit_price, stock, reorder_level FROM products";
    QStringList conditions;
    QVariantList binds;

    if (!filter.query.isEmpty()) {
        conditions << "(name LIKE ? OR sku LIKE ?)";
        const QString pattern = "%" + filter.query + "%";
        binds << pattern << pattern;
    }
    if (filter.lowStockOnly)
        conditions << "stock <= reorder_level";

    if (!conditions.isEmpty())
        sql += " WHERE " + conditions.join(" AND ");
    sql += " ORDER BY name COLLATE NOCASE";

    QSqlQuery q(m_db);
    q.prepare(sql);
    for (const auto &b : binds)
        q.addBindValue(b);

    std::vector<Product> out;
    if (!q.exec())
        return out;
    while (q.next())
        out.push_back(rowToProduct(q));
    return out;
}

bool ProductRepository::addStock(int id, int delta)
{
    QSqlQuery q(m_db);
    q.prepare("UPDATE products SET stock = stock + ? WHERE id=?");
    q.addBindValue(delta);
    q.addBindValue(id);
    if (!q.exec()) {
        m_lastError = q.lastError().text();
        return false;
    }
    return true;
}

} // namespace inv
