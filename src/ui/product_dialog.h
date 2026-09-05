#pragma once

#include "model/product.h"

#include <QDialog>

class QLineEdit;
class QDoubleSpinBox;
class QSpinBox;

namespace inv {

// Modal dialog for creating or editing a single product. Pre-fills fields
// from an existing Product, or starts empty for a new one.
class ProductDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProductDialog(QWidget *parent = nullptr);

    // Sets the values to edit; when not set, the dialog creates a new product.
    void setProduct(const Product &product);
    Product product() const;

private:
    void onAccepted();

    QLineEdit *m_name = nullptr;
    QLineEdit *m_sku = nullptr;
    QDoubleSpinBox *m_unitPrice = nullptr;
    QSpinBox *m_stock = nullptr;
    QSpinBox *m_reorder = nullptr;
    Product m_product;
};

} // namespace inv
