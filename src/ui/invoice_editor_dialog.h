#pragma once

#include "model/invoice.h"

#include <QComboBox>
#include <QDialog>

class QDateEdit;
class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QTableView;

namespace inv {

class Database;
class CustomerRepository;
class LineItemTableModel;

// Combo box of customers plus a trailing "New customer…" action that opens
// CustomerDialog inline. Exposes the chosen customer id and a way to refresh.
class CustomerComboBox : public QComboBox
{
    Q_OBJECT

public:
    explicit CustomerComboBox(Database &db, QWidget *parent = nullptr);

    void reload();
    int currentCustomerId() const;
    void selectCustomer(int id);

signals:
    void customerCreated(int id);

private:
    void onActivated(int index);

    Database &m_db;
    bool m_ignoreSelf = false;
};

// Full invoice editor: header (customer, dates, notes) + editable line items
// + live totals. Produces a filled Invoice on accept.
class InvoiceEditorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit InvoiceEditorDialog(Database &db, QWidget *parent = nullptr);

    // Pre-fills the editor with an existing invoice (edit mode).
    void setInvoice(const Invoice &invoice);
    // Returns the edited invoice on accept.
    Invoice invoice() const;

private:
    void setupUi();
    void refreshTotals();
    bool validate() const;
    void onAccept();

    Database &m_db;
    CustomerComboBox *m_customerCombo = nullptr;
    QDateEdit *m_issueDate = nullptr;
    QDateEdit *m_dueDate = nullptr;
    QLineEdit *m_number = nullptr;
    QPlainTextEdit *m_note = nullptr;
    QTableView *m_itemsView = nullptr;
    LineItemTableModel *m_itemsModel = nullptr;
    QLabel *m_subtotalLabel = nullptr;
    QLabel *m_taxLabel = nullptr;
    QLabel *m_totalLabel = nullptr;
    Invoice m_invoice;
};

} // namespace inv