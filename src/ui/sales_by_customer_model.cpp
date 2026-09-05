#include "sales_by_customer_model.h"

#include <QLocale>

namespace inv {

SalesByCustomerModel::SalesByCustomerModel(QSqlDatabase db, QObject *parent)
    : QAbstractTableModel(parent), m_repo(std::move(db))
{
}

int SalesByCustomerModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : static_cast<int>(m_rows.size());
}

int SalesByCustomerModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : ColCount;
}

QVariant SalesByCustomerModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return {};
    const auto &row = m_rows[index.row()];
    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case ColCustomer: return row.customerName;
        case ColRevenue: return QLocale().toCurrencyString(row.revenue);
        default: return {};
        }
    }
    if (role == Qt::TextAlignmentRole && index.column() == ColRevenue)
        return int(Qt::AlignRight | Qt::AlignVCenter);
    return {};
}

QVariant SalesByCustomerModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return QAbstractTableModel::headerData(section, orientation, role);
    switch (section) {
    case ColCustomer: return tr("Customer");
    case ColRevenue: return tr("Revenue");
    default: return {};
    }
}

void SalesByCustomerModel::refresh(const QString &from, const QString &to)
{
    beginResetModel();
    m_rows = m_repo.salesByCustomer(from, to);
    endResetModel();
}

} // namespace inv