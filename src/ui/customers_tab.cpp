#include "customers_tab.h"

#include "data/customer_repository.h"
#include "data/database.h"
#include "ui/customer_dialog.h"
#include "ui/customer_table_model.h"

#include <QAbstractItemView>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QPushButton>
#include <QTableView>
#include <QVBoxLayout>

namespace inv {

CustomersTab::CustomersTab(Database &db, QWidget *parent)
    : QWidget(parent), m_db(db)
{
    m_model = new CustomerTableModel(m_db.db(), this);

    m_view = new QTableView(this);
    m_view->setModel(m_model);
    m_view->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_view->setSelectionMode(QAbstractItemView::SingleSelection);
    m_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_view->verticalHeader()->setVisible(false);
    m_view->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_view->selectRow(0);

    auto *newBtn = new QPushButton(tr("New Customer"), this);
    auto *editBtn = new QPushButton(tr("Edit"), this);
    auto *delBtn = new QPushButton(tr("Delete"), this);
    connect(newBtn, &QPushButton::clicked, this, &CustomersTab::onNewCustomer);
    connect(editBtn, &QPushButton::clicked, this, &CustomersTab::onEditCustomer);
    connect(delBtn, &QPushButton::clicked, this, &CustomersTab::onDeleteCustomer);

    auto *btnRow = new QHBoxLayout;
    btnRow->addWidget(newBtn);
    btnRow->addWidget(editBtn);
    btnRow->addWidget(delBtn);
    btnRow->addStretch();

    auto *root = new QVBoxLayout(this);
    root->addLayout(btnRow);
    root->addWidget(m_view, 1);
}

void CustomersTab::onNewCustomer()
{
    CustomerDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted)
        return;

    CustomerRepository repo(m_db.db());
    if (!repo.create(dlg.customer())) {
        QMessageBox::warning(this, tr("Save failed"), repo.lastError());
        return;
    }
    refresh();
}

void CustomersTab::onEditCustomer()
{
    const auto idx = m_view->currentIndex();
    if (!idx.isValid())
        return;

    CustomerDialog dlg(this);
    dlg.setCustomer(m_model->customerAt(idx.row()));
    if (dlg.exec() != QDialog::Accepted)
        return;

    CustomerRepository repo(m_db.db());
    Customer c = dlg.customer();
    c.id = m_model->customerAt(idx.row()).id;
    if (!repo.update(c)) {
        QMessageBox::warning(this, tr("Save failed"), repo.lastError());
        return;
    }
    refresh();
}

void CustomersTab::onDeleteCustomer()
{
    const auto idx = m_view->currentIndex();
    if (!idx.isValid())
        return;

    const Customer c = m_model->customerAt(idx.row());
    const auto answer = QMessageBox::question(
        this, tr("Delete customer"),
        tr("Delete \"%1\"? Any invoices referencing this customer will keep their "
           "header but lose the linked customer name.")
            .arg(c.name));
    if (answer != QMessageBox::Yes)
        return;

    CustomerRepository repo(m_db.db());
    if (!repo.remove(c.id)) {
        QMessageBox::warning(this, tr("Delete failed"), repo.lastError());
        return;
    }
    refresh();
}

void CustomersTab::refresh()
{
    m_model->reload();
}

} // namespace inv