#pragma once

#include "data/invoice_repository.h"

#include <QAbstractTableModel>
#include <QSqlDatabase>

#include <vector>

namespace inv {

// Read-only model of invoice summaries for the "Invoices" tab.
class InvoiceListModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Column {
        ColNumber = 0,
        ColCustomer,
        ColIssueDate,
        ColTotal,
        ColCount
    };

    explicit InvoiceListModel(QSqlDatabase db, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = {}) const override;
    int columnCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    void reload();
    InvoiceRepository::Summary summaryAt(int row) const;
    int invoiceIdAt(int row) const { return m_rows[row].id; }

private:
    InvoiceRepository m_repo;
    std::vector<InvoiceRepository::Summary> m_rows;
};

} // namespace inv