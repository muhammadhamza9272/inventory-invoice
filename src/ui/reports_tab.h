#pragma once

#include <QWidget>

class QDateEdit;
class QLabel;
class QTableView;

namespace inv {

class Database;
class ProductTableModel;
class SalesByCustomerModel;

// Reports tab: sales summary for a date range + low-stock products.
class ReportsTab : public QWidget
{
    Q_OBJECT

public:
    explicit ReportsTab(Database &db, QWidget *parent = nullptr);

private:
    void refresh();

    Database &m_db;
    QDateEdit *m_from = nullptr;
    QDateEdit *m_to = nullptr;
    QLabel *m_invoicesLabel = nullptr;
    QLabel *m_revenueLabel = nullptr;
    QLabel *m_taxLabel = nullptr;
    QTableView *m_byCustomerView = nullptr;
    SalesByCustomerModel *m_byCustomerModel = nullptr;

    // Low-stock products reuse the existing all-columns product model.
    QTableView *m_lowStockView = nullptr;
    ProductTableModel *m_lowStockModel = nullptr;
};

} // namespace inv