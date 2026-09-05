#include "invoice_list_model.h"

#include <QLocale>

namespace inv {

InvoiceListModel::InvoiceListModel(QSqlDatabase db, QObject *parent)
    : QAbstractTableModel(parent), m_repo(std::move(db))
{
    reload();
}

int InvoiceListModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : static_cast<int>(m_rows.size());
}

int InvoiceListModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : ColCount;
}

QVariant InvoiceListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return {};
    const auto &row = m_rows[index.row()];

    switch (role) {
    case Qt::DisplayRole:
        switch (index.column()) {
        case ColNumber: return row.number;
        case ColCustomer: return row.customerName;
        case ColIssueDate: return row.issueDate;
        case ColTotal: return QLocale().toCurrencyString(row.total);
        default: return {};
        }
    case Qt::TextAlignmentRole:
        if (index.column() == ColTotal)
            return int(Qt::AlignRight | Qt::AlignVCenter);
        return int(Qt::AlignLeft | Qt::AlignVCenter);
    default:
        return {};
    }
}

QVariant InvoiceListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return QAbstractTableModel::headerData(section, orientation, role);
    switch (section) {
    case ColNumber: return tr("Number");
    case ColCustomer: return tr("Customer");
    case ColIssueDate: return tr("Issue Date");
    case ColTotal: return tr("Total");
    default: return {};
    }
}

void InvoiceListModel::reload()
{
    beginResetModel();
    m_rows = m_repo.listSummaries();
    endResetModel();
}

InvoiceRepository::Summary InvoiceListModel::summaryAt(int row) const
{
    return m_rows.at(row);
}

} // namespace inv