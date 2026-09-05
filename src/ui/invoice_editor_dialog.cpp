#include "invoice_editor_dialog.h"

#include "data/customer_repository.h"
#include "data/database.h"
#include "data/invoice_repository.h"
#include "ui/customer_dialog.h"
#include "ui/line_item_table_model.h"

#include <QAbstractItemView>
#include <QComboBox>
#include <QDate>
#include <QDateEdit>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QLocale>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTableView>
#include <QVBoxLayout>

namespace inv {

// ---------------------------------------------------------------- Combo box

CustomerComboBox::CustomerComboBox(Database &db, QWidget *parent)
    : QComboBox(parent), m_db(db)
{
    reload();
    connect(this, QOverload<int>::of(&QComboBox::activated),
            this, &CustomerComboBox::onActivated);
}

void CustomerComboBox::reload()
{
    m_ignoreSelf = true;
    clear();
    CustomerRepository repo(m_db.db());
    const auto customers = repo.list();
    for (const auto &c : customers)
        addItem(c.name, c.id);
    addItem(tr("+ New customer…"), -2);
    m_ignoreSelf = false;
}

void CustomerComboBox::onActivated(int index)
{
    if (m_ignoreSelf)
        return;
    if (itemData(index).toInt() == -2) {
        CustomerDialog dlg(this);
        if (dlg.exec() == QDialog::Accepted) {
            CustomerRepository repo(m_db.db());
            int id = -1;
            if (repo.create(dlg.customer(), &id)) {
                reload();
                selectCustomer(id);
                emit customerCreated(id);
                return;
            }
        }
        // Revert to nothing meaningful; leave selection where it was.
        setCurrentIndex(0);
    }
}

int CustomerComboBox::currentCustomerId() const
{
    return itemData(currentIndex()).toInt();
}

void CustomerComboBox::selectCustomer(int id)
{
    m_ignoreSelf = true;
    const int idx = findData(id);
    setCurrentIndex(idx >= 0 ? idx : 0);
    m_ignoreSelf = false;
}

// ------------------------------------------------------------ Editor dialog

InvoiceEditorDialog::InvoiceEditorDialog(Database &db, QWidget *parent)
    : QDialog(parent), m_db(db)
{
    setupUi();
    refreshTotals();
    connect(m_itemsModel, &LineItemTableModel::totalsChanged,
            this, &InvoiceEditorDialog::refreshTotals);
}

void InvoiceEditorDialog::setupUi()
{
    setWindowTitle(tr("Invoice"));
    setModal(true);
    resize(760, 560);

    auto *headerForm = new QFormLayout;

    m_number = new QLineEdit(this);
    m_number->setPlaceholderText(tr("Auto-generated if left empty"));
    headerForm->addRow(tr("Invoice #"), m_number);

    m_customerCombo = new CustomerComboBox(m_db, this);
    headerForm->addRow(tr("Customer"), m_customerCombo);

    m_issueDate = new QDateEdit(QDate::currentDate(), this);
    m_issueDate->setCalendarPopup(true);
    m_issueDate->setDisplayFormat("yyyy-MM-dd");
    headerForm->addRow(tr("Issue date"), m_issueDate);

    m_dueDate = new QDateEdit(QDate::currentDate().addDays(14), this);
    m_dueDate->setCalendarPopup(true);
    m_dueDate->setDisplayFormat("yyyy-MM-dd");
    headerForm->addRow(tr("Due date"), m_dueDate);

    m_note = new QPlainTextEdit(this);
    m_note->setFixedHeight(56);
    headerForm->addRow(tr("Note"), m_note);

    // Line items
    m_itemsModel = new LineItemTableModel(this);
    m_itemsView = new QTableView(this);
    m_itemsView->setModel(m_itemsModel);
    m_itemsView->setSelectionBehavior(QAbstractItemView::SelectItems);
    m_itemsView->setEditTriggers(QAbstractItemView::DoubleClicked
                                 | QAbstractItemView::EditKeyPressed
                                 | QAbstractItemView::AnyKeyPressed);
    m_itemsView->verticalHeader()->setVisible(false);
    m_itemsView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_itemsView->setColumnWidth(LineItemTableModel::ColQuantity, 80);
    m_itemsView->setColumnWidth(LineItemTableModel::ColUnitPrice, 110);
    m_itemsView->setColumnWidth(LineItemTableModel::ColTaxRate, 70);
    m_itemsView->setColumnWidth(LineItemTableModel::ColTotal, 120);

    auto *addItemBtn = new QPushButton(tr("Add line"), this);
    auto *removeItemBtn = new QPushButton(tr("Remove selected line"), this);
    connect(addItemBtn, &QPushButton::clicked, m_itemsModel,
            &LineItemTableModel::appendEmptyRow);
    connect(removeItemBtn, &QPushButton::clicked, this, [this] {
        const auto idx = m_itemsView->currentIndex();
        if (idx.isValid() && idx.row() < m_itemsModel->rowCount()) {
            m_itemsModel->removeRows(idx.row(), 1);
            refreshTotals();
        }
    });

    auto *itemBtnRow = new QHBoxLayout;
    itemBtnRow->addWidget(addItemBtn);
    itemBtnRow->addWidget(removeItemBtn);
    itemBtnRow->addStretch();

    // Totals
    m_subtotalLabel = new QLabel(this);
    m_taxLabel = new QLabel(this);
    m_totalLabel = new QLabel(this);
    auto *totalsForm = new QFormLayout;
    totalsForm->addRow(tr("Subtotal"), m_subtotalLabel);
    totalsForm->addRow(tr("Tax"), m_taxLabel);
    auto *totalRow = new QHBoxLayout;
    auto *totalCaption = new QLabel(tr("<b>Grand total</b>"), this);
    m_totalLabel->setStyleSheet("font-weight: bold; font-size: 16px;");
    totalRow->addWidget(totalCaption);
    totalRow->addWidget(m_totalLabel, 0, Qt::AlignRight);

    // Footer buttons
    auto *save = new QPushButton(tr("Save"), this);
    auto *cancel = new QPushButton(tr("Cancel"), this);
    save->setDefault(true);
    connect(save, &QPushButton::clicked, this, &InvoiceEditorDialog::onAccept);
    connect(cancel, &QPushButton::clicked, this, &QDialog::reject);
    auto *btnRow = new QHBoxLayout;
    btnRow->addStretch();
    btnRow->addWidget(cancel);
    btnRow->addWidget(save);

    auto *root = new QVBoxLayout(this);
    root->addLayout(headerForm);
    root->addWidget(m_itemsView, 1);
    root->addLayout(itemBtnRow);
    root->addLayout(totalsForm);
    root->addLayout(totalRow);
    root->addLayout(btnRow);
}

void InvoiceEditorDialog::setInvoice(const Invoice &invoice)
{
    m_invoice = invoice;
    setWindowTitle(invoice.id < 0 ? tr("New Invoice") : tr("Edit Invoice"));
    m_number->setText(invoice.number);
    if (invoice.customerId >= 0)
        m_customerCombo->selectCustomer(invoice.customerId);
    if (!invoice.issueDate.isEmpty())
        m_issueDate->setDate(QDate::fromString(invoice.issueDate, "yyyy-MM-dd"));
    if (!invoice.dueDate.isEmpty())
        m_dueDate->setDate(QDate::fromString(invoice.dueDate, "yyyy-MM-dd"));
    m_note->setPlainText(invoice.note);
    m_itemsModel->setItems(invoice.items);
    refreshTotals();
}

Invoice InvoiceEditorDialog::invoice() const
{
    return m_invoice;
}

void InvoiceEditorDialog::refreshTotals()
{
    m_subtotalLabel->setText(QLocale().toCurrencyString(m_itemsModel->subtotal()));
    m_taxLabel->setText(QLocale().toCurrencyString(m_itemsModel->totalTax()));
    m_totalLabel->setText(QLocale().toCurrencyString(m_itemsModel->total()));
}

bool InvoiceEditorDialog::validate() const
{
    if (m_customerCombo->currentCustomerId() <= 0) {
        QMessageBox::warning(const_cast<InvoiceEditorDialog *>(this), tr("Missing customer"),
                             tr("Select a customer before saving."));
        m_customerCombo->setFocus();
        return false;
    }
    for (const auto &item : m_itemsModel->items()) {
        if (item.description.trimmed().isEmpty() || item.quantity <= 0.0) {
            QMessageBox::warning(const_cast<InvoiceEditorDialog *>(this), tr("Incomplete line"),
                                 tr("Every line needs a description and a quantity above zero."));
            return false;
        }
    }
    return true;
}

void InvoiceEditorDialog::onAccept()
{
    if (!validate())
        return;

    m_invoice.number = m_number->text().trimmed();
    m_invoice.customerId = m_customerCombo->currentCustomerId();
    m_invoice.issueDate = m_issueDate->date().toString("yyyy-MM-dd");
    m_invoice.dueDate = m_dueDate->date().toString("yyyy-MM-dd");
    m_invoice.note = m_note->toPlainText().trimmed();
    m_invoice.items = m_itemsModel->items();

    accept();
}

} // namespace inv