#include "customer_table_model.h"

namespace inv {

CustomerTableModel::CustomerTableModel(QSqlDatabase db, QObject *parent)
    : QAbstractTableModel(parent), m_repo(std::move(db))
{
    reload();
}

void CustomerTableModel::reload()
{
    beginResetModel();
    m_rows = m_repo.list();
    endResetModel();
}

int CustomerTableModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : static_cast<int>(m_rows.size());
}

int CustomerTableModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : ColCount;
}

QVariant CustomerTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return {};
    const Customer &c = m_rows[index.row()];
    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case ColName: return c.name;
        case ColEmail: return c.email;
        case ColPhone: return c.phone;
        case ColAddress: return c.billingAddress;
        default: return {};
        }
    }
    return {};
}

QVariant CustomerTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return QAbstractTableModel::headerData(section, orientation, role);
    switch (section) {
    case ColName: return tr("Name");
    case ColEmail: return tr("Email");
    case ColPhone: return tr("Phone");
    case ColAddress: return tr("Billing Address");
    default: return {};
    }
}

const Customer &CustomerTableModel::customerAt(int row) const
{
    return m_rows.at(row);
}

} // namespace inv