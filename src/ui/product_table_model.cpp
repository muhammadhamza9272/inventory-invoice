#include "product_table_model.h"

#include <QColor>
#include <QLocale>

namespace inv {

ProductTableModel::ProductTableModel(QSqlDatabase db, QObject *parent)
    : QAbstractTableModel(parent), m_repo(std::move(db))
{
    reload();
}

int ProductTableModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : static_cast<int>(m_rows.size());
}

int ProductTableModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : ColCount;
}

QVariant ProductTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return {};
    const Product &p = m_rows[index.row()];

    switch (role) {
    case Qt::DisplayRole:
        switch (index.column()) {
        case ColName: return p.name;
        case ColSku: return p.sku;
        case ColUnitPrice: return QLocale().toCurrencyString(p.unitPrice);
        case ColStock: return p.stock;
        case ColReorder: return p.reorderLevel;
        default: return {};
        }
    case Qt::TextAlignmentRole:
        if (index.column() == ColUnitPrice || index.column() == ColStock)
            return int(Qt::AlignRight | Qt::AlignVCenter);
        return int(Qt::AlignLeft | Qt::AlignVCenter);
    case Qt::ForegroundRole:
        if (index.column() == ColStock && p.stock <= p.reorderLevel)
            return QColor(Qt::red);
        return {};
    case Qt::ToolTipRole:
        if (index.column() == ColStock && p.stock <= p.reorderLevel)
            return tr("Low stock — at or below reorder level");
        return {};
    default:
        return {};
    }
}

QVariant ProductTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return QAbstractTableModel::headerData(section, orientation, role);
    switch (section) {
    case ColName: return tr("Name");
    case ColSku: return tr("SKU");
    case ColUnitPrice: return tr("Unit Price");
    case ColStock: return tr("Stock");
    case ColReorder: return tr("Reorder At");
    default: return {};
    }
}

Qt::ItemFlags ProductTableModel::flags(const QModelIndex &index) const
{
    return QAbstractTableModel::flags(index); // read-only view
}

const Product &ProductTableModel::productAt(int row) const
{
    return m_rows.at(row);
}

void ProductTableModel::setFilter(const ProductFilter &filter)
{
    m_lastFilter = filter;
    beginResetModel();
    m_rows = m_repo.list(filter);
    endResetModel();
}

void ProductTableModel::refreshOne(int productId)
{
    for (int i = 0; i < int(m_rows.size()); ++i) {
        if (m_rows[i].id == productId) {
            if (auto p = m_repo.findById(productId))
                m_rows[i] = *p;
            emit dataChanged(index(i, 0), index(i, ColCount - 1));
            return;
        }
    }
    refreshAll(); // not visible already; just reload
}

void ProductTableModel::refreshAll()
{
    setFilter(m_lastFilter);
}

void ProductTableModel::reload()
{
    setFilter(ProductFilter{});
}

} // namespace inv
