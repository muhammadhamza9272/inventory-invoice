#pragma once

#include "data/customer_repository.h"

#include <QAbstractTableModel>
#include <QSqlDatabase>

#include <vector>

namespace inv {

// Read-only model of customers for the Customers tab.
class CustomerTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Column {
        ColName = 0,
        ColEmail,
        ColPhone,
        ColAddress,
        ColCount
    };

    explicit CustomerTableModel(QSqlDatabase db, QObject *parent = nullptr);

    void reload();

    int rowCount(const QModelIndex &parent = {}) const override;
    int columnCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    const Customer &customerAt(int row) const;

private:
    CustomerRepository m_repo;
    std::vector<Customer> m_rows;
};

} // namespace inv