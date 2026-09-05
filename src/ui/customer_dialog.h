#pragma once

#include "model/invoice.h"

#include <QDialog>

class QLineEdit;

namespace inv {

// Modal dialog for creating or editing one customer.
class CustomerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CustomerDialog(QWidget *parent = nullptr);

    void setCustomer(const Customer &customer);
    Customer customer() const;

private:
    void onAccepted();

    QLineEdit *m_name = nullptr;
    QLineEdit *m_email = nullptr;
    QLineEdit *m_phone = nullptr;
    QLineEdit *m_address = nullptr;
    Customer m_customer;
};

} // namespace inv