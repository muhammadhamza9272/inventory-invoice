#include "product_dialog.h"

#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QLocale>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

namespace inv {

ProductDialog::ProductDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Product"));
    setModal(true);
    setMinimumWidth(360);

    m_name = new QLineEdit(this);
    m_sku = new QLineEdit(this);
    m_unitPrice = new QDoubleSpinBox(this);
    m_unitPrice->setPrefix(QLocale().currencySymbol() + " ");
    m_unitPrice->setRange(0.0, 99999999.0);
    m_unitPrice->setDecimals(2);
    m_stock = new QSpinBox(this);
    m_stock->setRange(0, 1000000);
    m_reorder = new QSpinBox(this);
    m_reorder->setRange(0, 1000000);

    auto *form = new QFormLayout;
    form->addRow(tr("Name"), m_name);
    form->addRow(tr("SKU"), m_sku);
    form->addRow(tr("Unit Price"), m_unitPrice);
    form->addRow(tr("Stock"), m_stock);
    form->addRow(tr("Reorder At"), m_reorder);

    auto *save = new QPushButton(tr("Save"), this);
    auto *cancel = new QPushButton(tr("Cancel"), this);
    save->setDefault(true);
    connect(save, &QPushButton::clicked, this, &ProductDialog::onAccepted);
    connect(cancel, &QPushButton::clicked, this, &QDialog::reject);

    auto *btnRow = new QHBoxLayout;
    btnRow->addStretch();
    btnRow->addWidget(cancel);
    btnRow->addWidget(save);

    auto *root = new QVBoxLayout(this);
    root->addLayout(form);
    root->addLayout(btnRow);
}

void ProductDialog::setProduct(const Product &product)
{
    m_product = product;
    m_name->setText(product.name);
    m_sku->setText(product.sku);
    m_unitPrice->setValue(product.unitPrice);
    m_stock->setValue(product.stock);
    m_reorder->setValue(product.reorderLevel);
    setWindowTitle(product.id < 0 ? tr("New Product") : tr("Edit Product"));
}

Product ProductDialog::product() const
{
    return m_product;
}

void ProductDialog::onAccepted()
{
    if (m_name->text().trimmed().isEmpty()) {
        m_name->setFocus();
        return; // keep dialog open; host validates
    }
    m_product.name = m_name->text().trimmed();
    m_product.sku = m_sku->text().trimmed();
    m_product.unitPrice = m_unitPrice->value();
    m_product.stock = m_stock->value();
    m_product.reorderLevel = m_reorder->value();
    accept();
}

} // namespace inv
