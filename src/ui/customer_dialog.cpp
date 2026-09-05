#include "customer_dialog.h"

#include <QFormLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

namespace inv {

CustomerDialog::CustomerDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Customer"));
    setModal(true);
    setMinimumWidth(380);

    m_name = new QLineEdit(this);
    m_email = new QLineEdit(this);
    m_email->setPlaceholderText(tr("name@example.com"));
    m_phone = new QLineEdit(this);
    m_address = new QLineEdit(this);

    auto *form = new QFormLayout;
    form->addRow(tr("Name"), m_name);
    form->addRow(tr("Email"), m_email);
    form->addRow(tr("Phone"), m_phone);
    form->addRow(tr("Billing Address"), m_address);

    auto *save = new QPushButton(tr("Save"), this);
    auto *cancel = new QPushButton(tr("Cancel"), this);
    save->setDefault(true);
    connect(save, &QPushButton::clicked, this, &CustomerDialog::onAccepted);
    connect(cancel, &QPushButton::clicked, this, &QDialog::reject);

    auto *btnRow = new QHBoxLayout;
    btnRow->addStretch();
    btnRow->addWidget(cancel);
    btnRow->addWidget(save);

    auto *root = new QVBoxLayout(this);
    root->addLayout(form);
    root->addLayout(btnRow);
}

void CustomerDialog::setCustomer(const Customer &customer)
{
    m_customer = customer;
    m_name->setText(customer.name);
    m_email->setText(customer.email);
    m_phone->setText(customer.phone);
    m_address->setText(customer.billingAddress);
    setWindowTitle(customer.id < 0 ? tr("New Customer") : tr("Edit Customer"));
}

Customer CustomerDialog::customer() const
{
    return m_customer;
}

void CustomerDialog::onAccepted()
{
    if (m_name->text().trimmed().isEmpty()) {
        m_name->setFocus();
        return;
    }
    m_customer.name = m_name->text().trimmed();
    m_customer.email = m_email->text().trimmed();
    m_customer.phone = m_phone->text().trimmed();
    m_customer.billingAddress = m_address->text().trimmed();
    accept();
}

} // namespace inv