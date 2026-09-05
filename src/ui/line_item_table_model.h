#pragma once

#include "model/invoice.h"

#include <QAbstractTableModel>

#include <vector>

namespace inv {

// Editable table model for the invoice's line items. Each row is a LineItem;
// edits are stored directly into the owned items vector. View/form widgets
// set the LineItem fields per-column.
class LineItemTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Column {
        ColDescription = 0,
        ColQuantity,
        ColUnitPrice,
        ColTaxRate,
        ColTotal,
        ColCount
    };

    explicit LineItemTableModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = {}) const override;
    int columnCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    void setItems(const std::vector<LineItem> &items);
    std::vector<LineItem> items() const { return m_items; }
    void appendEmptyRow();
    bool removeRows(int row, int count, const QModelIndex &parent = {}) override;

    // Totals (kept up to date with current item values).
    double subtotal() const;
    double totalTax() const;
    double total() const;

signals:
    void totalsChanged();

private:
    void emitTotals();

    std::vector<LineItem> m_items;
};

} // namespace inv
