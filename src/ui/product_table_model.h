#pragma once

#include "data/product_repository.h"
#include "model/product.h"

#include <QAbstractTableModel>
#include <QSqlDatabase>

#include <vector>

namespace inv {

// Read-only table model exposing products to a QTableView. Data is loaded
// from the repository on demand; mutations go through ProductRepository.
class ProductTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Column {
        ColName = 0,
        ColSku,
        ColUnitPrice,
        ColStock,
        ColReorder,
        ColCount
    };

    explicit ProductTableModel(QSqlDatabase db, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = {}) const override;
    int columnCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    // Reload rows matching filter from the repository.
    void setFilter(const ProductFilter &filter);
    const Product &productAt(int row) const;

    // Refreshes a single row (or reloads all if not found).
    void refreshOne(int productId);
    void refreshAll();

private:
    void reload();

    ProductRepository m_repo;
    std::vector<Product> m_rows;
    ProductFilter m_lastFilter;
};

} // namespace inv
