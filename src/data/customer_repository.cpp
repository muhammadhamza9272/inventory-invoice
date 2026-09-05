#include "customer_repository.h"

#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

namespace inv {

namespace {
// A default-constructed QString binds as SQL NULL; empty-but-not-null is what
// our NOT NULL columns expect.
QString nonNull(const QString &s) { return s.isEmpty() ? QStringLiteral("") : s; }
}

Customer CustomerRepository::rowToCustomer(const QSqlQuery &q) const
{
    Customer c;
    c.id = q.value(0).toInt();
    c.name = q.value(1).toString();
    c.email = q.value(2).toString();
    c.phone = q.value(3).toString();
    c.billingAddress = q.value(4).toString();
    return c;
}

bool CustomerRepository::create(const Customer &customer, int *outId)
{
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO customers (name, email, phone, billing_address) VALUES (?, ?, ?, ?)");
    q.addBindValue(nonNull(customer.name));
    q.addBindValue(nonNull(customer.email));
    q.addBindValue(nonNull(customer.phone));
    q.addBindValue(nonNull(customer.billingAddress));
    if (!q.exec()) {
        m_lastError = q.lastError().text();
        return false;
    }
    if (outId)
        *outId = q.lastInsertId().toInt();
    return true;
}

bool CustomerRepository::update(const Customer &customer)
{
    QSqlQuery q(m_db);
    q.prepare("UPDATE customers SET name=?, email=?, phone=?, billing_address=? WHERE id=?");
    q.addBindValue(nonNull(customer.name));
    q.addBindValue(nonNull(customer.email));
    q.addBindValue(nonNull(customer.phone));
    q.addBindValue(nonNull(customer.billingAddress));
    q.addBindValue(customer.id);
    if (!q.exec()) {
        m_lastError = q.lastError().text();
        return false;
    }
    return true;
}

bool CustomerRepository::remove(int id)
{
    QSqlQuery q(m_db);
    q.prepare("DELETE FROM customers WHERE id=?");
    q.addBindValue(id);
    if (!q.exec()) {
        m_lastError = q.lastError().text();
        return false;
    }
    return true;
}

std::optional<Customer> CustomerRepository::findById(int id) const
{
    QSqlQuery q(m_db);
    q.prepare("SELECT id, name, email, phone, billing_address FROM customers WHERE id=?");
    q.addBindValue(id);
    if (!q.exec() || !q.next())
        return std::nullopt;
    return rowToCustomer(q);
}

std::vector<Customer> CustomerRepository::list(const QString &query) const
{
    QString sql = "SELECT id, name, email, phone, billing_address FROM customers";
    QVariantList binds;

    if (!query.isEmpty()) {
        sql += " WHERE name LIKE ? OR email LIKE ? OR phone LIKE ?";
        const QString pattern = "%" + query + "%";
        binds << pattern << pattern << pattern;
    }
    sql += " ORDER BY name COLLATE NOCASE";

    QSqlQuery q(m_db);
    q.prepare(sql);
    for (const auto &b : binds)
        q.addBindValue(b);

    std::vector<Customer> out;
    if (!q.exec())
        return out;
    while (q.next())
        out.push_back(rowToCustomer(q));
    return out;
}

int CustomerRepository::count() const
{
    QSqlQuery q(m_db);
    if (q.exec("SELECT COUNT(*) FROM customers") && q.next())
        return q.value(0).toInt();
    return 0;
}

} // namespace inv
