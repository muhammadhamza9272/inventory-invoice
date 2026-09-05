#include "line_item_table_model.h"

#include <QLocale>

namespace inv {

LineItemTableModel::LineItemTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

int LineItemTableModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : static_cast<int>(m_items.size());
}

int LineItemTableModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : ColCount;
}

QVariant LineItemTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return {};
    const LineItem &item = m_items[index.row()];

    switch (role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
        switch (index.column()) {
        case ColDescription: return item.description;
        case ColQuantity: return item.quantity;
        case ColUnitPrice: return item.unitPrice;
        case ColTaxRate: return item.taxRate;
        case ColTotal: return QLocale().toCurrencyString(item.lineTotalWithTax());
        default: return {};
        }
    case Qt::TextAlignmentRole:
        if (index.column() != ColDescription)
            return int(Qt::AlignRight | Qt::AlignVCenter);
        return int(Qt::AlignLeft | Qt::AlignVCenter);
    default:
        return {};
    }
}

QVariant LineItemTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return QAbstractTableModel::headerData(section, orientation, role);
    switch (section) {
    case ColDescription: return tr("Description");
    case ColQuantity: return tr("Qty");
    case ColUnitPrice: return tr("Unit Price");
    case ColTaxRate: return tr("Tax %");
    case ColTotal: return tr("Total");
    default: return {};
    }
}

Qt::ItemFlags LineItemTableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;
    if (index.column() == ColTotal)
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable; // computed, not editable
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

bool LineItemTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    LineItem &item = m_items[index.row()];
    switch (index.column()) {
    case ColDescription:
        item.description = value.toString();
        break;
    case ColQuantity:
        item.quantity = value.toDouble();
        break;
    case ColUnitPrice:
        item.unitPrice = value.toDouble();
        break;
    case ColTaxRate:
        item.taxRate = value.toDouble();
        break;
    default:
        return false;
    }

    emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});
    // The total column for this row and the grand totals both changed.
    emit dataChanged(index.siblingAtColumn(ColTotal), index.siblingAtColumn(ColTotal),
                     {Qt::DisplayRole});
    emitTotals();
    return true;
}

void LineItemTableModel::setItems(const std::vector<LineItem> &items)
{
    beginResetModel();
    m_items = items;
    endResetModel();
    emitTotals();
}

void LineItemTableModel::appendEmptyRow()
{
    beginInsertRows({}, m_items.size(), m_items.size());
    m_items.push_back(LineItem{});
    endInsertRows();
    emitTotals();
}

bool LineItemTableModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (parent.isValid() || row < 0 || row + count > int(m_items.size()))
        return false;
    beginRemoveRows({}, row, row + count - 1);
    m_items.erase(m_items.begin() + row, m_items.begin() + row + count);
    endRemoveRows();
    emitTotals();
    return true;
}

void LineItemTableModel::emitTotals()
{
    emit totalsChanged();
}

double LineItemTableModel::subtotal() const
{
    double sum = 0.0;
    for (const auto &item : m_items)
        sum += item.lineTotal();
    return sum;
}

double LineItemTableModel::totalTax() const
{
    double sum = 0.0;
    for (const auto &item : m_items)
        sum += item.taxAmount();
    return sum;
}

double LineItemTableModel::total() const
{
    double sum = 0.0;
    for (const auto &item : m_items)
        sum += item.lineTotalWithTax();
    return sum;
}

} // namespace inv
