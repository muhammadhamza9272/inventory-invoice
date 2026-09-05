#pragma once

#include "data/invoice_repository.h"

#include <QAbstractTableModel>
#include <QSqlDatabase>

#include <vector>

namespace inv {

// Read-only model of revenue grouped by customer for a date range.
class SalesByCustomerModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Column {
        ColCustomer = 0,
        ColRevenue,
        ColCount
    };

    explicit SalesByCustomerModel(QSqlDatabase db, QObject *parent = nullptr);

    void refresh(const QString &from, const QString &to);

    int rowCount(const QModelIndex &parent = {}) const override;
    int columnCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

private:
    InvoiceRepository m_repo;
    std::vector<InvoiceRepository::CustomerSales> m_rows;
};

} // namespace inv