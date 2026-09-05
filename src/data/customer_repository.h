#pragma once

#include "model/invoice.h"

#include <QSqlDatabase>

#include <optional>
#include <vector>

namespace inv {

class CustomerRepository
{
public:
    explicit CustomerRepository(QSqlDatabase db) : m_db(std::move(db)) {}

    bool create(const Customer &customer, int *outId = nullptr);
    bool update(const Customer &customer);
    bool remove(int id);

    std::optional<Customer> findById(int id) const;
    std::vector<Customer> list(const QString &query = {}) const;
    int count() const;

    QString lastError() const { return m_lastError; }

private:
    Customer rowToCustomer(const QSqlQuery &q) const;

    QSqlDatabase m_db;
    mutable QString m_lastError;
};

} // namespace inv
