#pragma once

#include <QCheckBox>
#include <QMainWindow>
#include <QTableView>

namespace inv {

class Database;
class ProductTableModel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(Database &db, int initialTab = 0, QWidget *parent = nullptr);
    void openInvoiceEditorForNewInvoice();
    void openInvoiceEditorForInvoice(int invoiceId);

private:
    void setupUi();
    void buildInventoryActions();

    void onAddProduct();
    void onEditProduct();
    void onDeleteProduct();
    void onReceiveStock();
    void onAdjustStock();
    void onSearchChanged(const QString &text);
    void onToggleLowStock(bool checked);

    void adjustStockDialog(int current, const QString &title);

    Database &m_db;
    ProductTableModel *m_model = nullptr;
    QTableView *m_view = nullptr;
    QCheckBox *m_lowStockCheck = nullptr;
    QTabWidget *m_tabs = nullptr;
};

} // namespace inv
