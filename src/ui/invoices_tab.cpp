#include "invoices_tab.h"

#include "data/customer_repository.h"
#include "data/database.h"
#include "data/invoice_repository.h"
#include "ui/invoice_editor_dialog.h"
#include "ui/invoice_list_model.h"
#include "ui/invoice_pdf_exporter.h"

#include <QAbstractItemView>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QPushButton>
#include <QTableView>
#include <QVBoxLayout>

namespace inv {

InvoicesTab::InvoicesTab(Database &db, QWidget *parent)
    : QWidget(parent), m_db(db)
{
    m_model = new InvoiceListModel(m_db.db(), this);

    m_view = new QTableView(this);
    m_view->setModel(m_model);
    m_view->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_view->setSelectionMode(QAbstractItemView::SingleSelection);
    m_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_view->verticalHeader()->setVisible(false);
    m_view->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_view->selectRow(0);

    auto *newBtn = new QPushButton(tr("New Invoice"), this);
    auto *editBtn = new QPushButton(tr("Edit"), this);
    auto *delBtn = new QPushButton(tr("Delete"), this);
    auto *pdfBtn = new QPushButton(tr("Export PDF"), this);
    connect(newBtn, &QPushButton::clicked, this, &InvoicesTab::onNewInvoice);
    connect(editBtn, &QPushButton::clicked, this, &InvoicesTab::onEditInvoice);
    connect(delBtn, &QPushButton::clicked, this, &InvoicesTab::onDeleteInvoice);
    connect(pdfBtn, &QPushButton::clicked, this, &InvoicesTab::onExportPdf);

    auto *btnRow = new QHBoxLayout;
    btnRow->addWidget(newBtn);
    btnRow->addWidget(editBtn);
    btnRow->addWidget(delBtn);
    btnRow->addWidget(pdfBtn);
    btnRow->addStretch();

    auto *root = new QVBoxLayout(this);
    root->addLayout(btnRow);
    root->addWidget(m_view, 1);
}

int InvoicesTab::selectedInvoiceId() const
{
    const auto idx = m_view->currentIndex();
    return idx.isValid() ? m_model->invoiceIdAt(idx.row()) : -1;
}

void InvoicesTab::onNewInvoice()
{
    InvoiceEditorDialog dlg(m_db, this);
    if (dlg.exec() != QDialog::Accepted)
        return;

    InvoiceRepository repo(m_db.db());
    Invoice inv = dlg.invoice();
    if (!repo.create(inv)) {
        QMessageBox::warning(this, tr("Save failed"), repo.lastError());
        return;
    }
    m_model->reload();
    // select the new row (last)
    m_view->selectRow(0);
}

void InvoicesTab::openNewInvoiceDialog()
{
    onNewInvoice();
}

void InvoicesTab::onEditInvoice()
{
    const int id = selectedInvoiceId();
    if (id < 0)
        return;

    InvoiceRepository repo(m_db.db());
    auto inv = repo.findById(id);
    if (!inv) {
        QMessageBox::warning(this, tr("Load failed"), repo.lastError());
        return;
    }

    InvoiceEditorDialog dlg(m_db, this);
    dlg.setInvoice(*inv);
    if (dlg.exec() != QDialog::Accepted)
        return;

    Invoice updated = dlg.invoice();
    updated.id = id;
    if (!repo.update(updated)) {
        QMessageBox::warning(this, tr("Save failed"), repo.lastError());
        return;
    }
    m_model->reload();
}

void InvoicesTab::onDeleteInvoice()
{
    const int id = selectedInvoiceId();
    if (id < 0)
        return;

    const auto answer = QMessageBox::question(
        this, tr("Delete invoice"),
        tr("Delete invoice %1? This cannot be undone.")
            .arg(m_model->summaryAt(m_view->currentIndex().row()).number));
    if (answer != QMessageBox::Yes)
        return;

    InvoiceRepository repo(m_db.db());
    if (!repo.remove(id)) {
        QMessageBox::warning(this, tr("Delete failed"), repo.lastError());
        return;
    }
    m_model->reload();
}

void InvoicesTab::onExportPdf()
{
    const int id = selectedInvoiceId();
    if (id < 0)
        return;

    InvoiceRepository repo(m_db.db());
    auto inv = repo.findById(id);
    if (!inv)
        return;

    CustomerRepository custRepo(m_db.db());
    auto customer = custRepo.findById(inv->customerId);
    if (!customer) {
        QMessageBox::warning(this, tr("Export failed"),
                             tr("The invoice's customer could not be found."));
        return;
    }

    const QString start = InvoicePdfExporter::suggestedFileName(*inv);
    const QString filePath = QFileDialog::getSaveFileName(
        this, tr("Export PDF"), start, tr("PDF files (*.pdf)"));
    if (filePath.isEmpty())
        return;

    QString err;
    if (!InvoicePdfExporter().exportTo(*inv, *customer, filePath, &err)) {
        QMessageBox::warning(this, tr("Export failed"),
                             err.isEmpty() ? tr("Unknown error") : err);
        return;
    }
}

} // namespace inv