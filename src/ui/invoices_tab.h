#pragma once

#include <QWidget>

class QTableView;

namespace inv {

class Database;
class InvoiceListModel;

// Invoices tab: list of saved invoices with New / Edit / Delete / Export PDF.
class InvoicesTab : public QWidget
{
    Q_OBJECT

public:
    explicit InvoicesTab(Database &db, QWidget *parent = nullptr);
    void openNewInvoiceDialog();
    // Opens an existing invoice for editing (used by the --edit-invoice flag).
    void openEditInvoiceDialog(int invoiceId);

private:
    void onNewInvoice();
    void onEditInvoice();
    void onDeleteInvoice();
    void onExportPdf();

    int selectedInvoiceId() const;

    Database &m_db;
    InvoiceListModel *m_model = nullptr;
    QTableView *m_view = nullptr;
};

} // namespace inv