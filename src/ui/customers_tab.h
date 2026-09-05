#pragma once

#include <QWidget>

class QTableView;

namespace inv {

class Database;
class CustomerTableModel;

// Customers tab: list of customers with New / Edit / Delete.
class CustomersTab : public QWidget
{
    Q_OBJECT

public:
    explicit CustomersTab(Database &db, QWidget *parent = nullptr);

private:
    void onNewCustomer();
    void onEditCustomer();
    void onDeleteCustomer();

    void refresh();

    Database &m_db;
    CustomerTableModel *m_model = nullptr;
    QTableView *m_view = nullptr;
};

} // namespace inv