#include "main_window.h"

#include "data/database.h"
#include "data/product_repository.h"
#include "ui/customers_tab.h"
#include "ui/invoices_tab.h"
#include "ui/product_dialog.h"
#include "ui/product_table_model.h"
#include "ui/reports_tab.h"

#include <QAbstractItemView>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTabWidget>
#include <QTableView>
#include <QVBoxLayout>
#include <QWidget>

using namespace inv;

namespace {

Product selectedProduct(ProductTableModel *model, const QModelIndex &index)
{
    if (index.isValid())
        return model->productAt(index.row());
    return {};
}

} // namespace

MainWindow::MainWindow(Database &db, int initialTab, QWidget *parent)
    : QMainWindow(parent), m_db(db)
{
    setupUi();
    setWindowTitle(tr("Inventory & Invoice"));
    resize(920, 560);
    m_tabs->setCurrentIndex(qBound(0, initialTab, m_tabs->count() - 1));
}

void MainWindow::setupUi()
{
    // ---- Inventory tab ----
    auto *inventoryTab = new QWidget(this);
    auto *invLayout = new QVBoxLayout(inventoryTab);

    m_view = new QTableView(inventoryTab);
    m_model = new ProductTableModel(m_db.db(), this);
    m_view->setModel(m_model);
    m_view->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_view->setSelectionMode(QAbstractItemView::SingleSelection);
    m_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_view->verticalHeader()->setVisible(false);
    m_view->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_view->horizontalHeader()->setSectionResizeMode(ProductTableModel::ColSku,
                                                     QHeaderView::ResizeToContents);
    m_view->horizontalHeader()->setSectionResizeMode(ProductTableModel::ColStock,
                                                     QHeaderView::ResizeToContents);
    m_view->selectRow(0);

    // Inventory actions (toolbar)
    auto *invToolbar = new QWidget(inventoryTab);
    auto *invToolbarRow = new QHBoxLayout(invToolbar);
    invToolbarRow->setContentsMargins(0, 0, 0, 0);
    auto *addBtn = new QPushButton(tr("Add"), invToolbar);
    auto *editBtn = new QPushButton(tr("Edit"), invToolbar);
    auto *delBtn = new QPushButton(tr("Delete"), invToolbar);
    auto *receiveBtn = new QPushButton(tr("Receive Stock"), invToolbar);
    auto *adjustBtn = new QPushButton(tr("Adjust Stock"), invToolbar);
    connect(addBtn, &QPushButton::clicked, this, &MainWindow::onAddProduct);
    connect(editBtn, &QPushButton::clicked, this, &MainWindow::onEditProduct);
    connect(delBtn, &QPushButton::clicked, this, &MainWindow::onDeleteProduct);
    connect(receiveBtn, &QPushButton::clicked, this, &MainWindow::onReceiveStock);
    connect(adjustBtn, &QPushButton::clicked, this, &MainWindow::onAdjustStock);
    invToolbarRow->addWidget(addBtn);
    invToolbarRow->addWidget(editBtn);
    invToolbarRow->addWidget(delBtn);
    invToolbarRow->addStretch();
    invToolbarRow->addWidget(receiveBtn);
    invToolbarRow->addWidget(adjustBtn);

    // Search row
    auto *search = new QLineEdit(inventoryTab);
    search->setPlaceholderText(tr("Search name or SKU…"));
    connect(search, &QLineEdit::textChanged, this, &MainWindow::onSearchChanged);

    m_lowStockCheck = new QCheckBox(tr("Low stock only"), inventoryTab);
    connect(m_lowStockCheck, &QCheckBox::toggled, this, &MainWindow::onToggleLowStock);

    auto *searchRow = new QHBoxLayout;
    searchRow->addWidget(search, 1);
    searchRow->addWidget(m_lowStockCheck);

    invLayout->addWidget(invToolbar);
    invLayout->addLayout(searchRow);
    invLayout->addWidget(m_view, 1);

    // ---- Tabs ----
    m_tabs = new QTabWidget(this);
    m_tabs->addTab(inventoryTab, tr("Inventory"));
    m_tabs->addTab(new InvoicesTab(m_db, this), tr("Invoices"));
    m_tabs->addTab(new CustomersTab(m_db, this), tr("Customers"));
    m_tabs->addTab(new ReportsTab(m_db, this), tr("Reports"));
    setCentralWidget(m_tabs);
}

void MainWindow::openInvoiceEditorForNewInvoice()
{
    m_tabs->setCurrentIndex(1); // Invoices tab
    auto *tab = qobject_cast<InvoicesTab *>(m_tabs->widget(1));
    if (tab)
        tab->openNewInvoiceDialog();
}

void MainWindow::openInvoiceEditorForInvoice(int invoiceId)
{
    m_tabs->setCurrentIndex(1); // Invoices tab
    auto *tab = qobject_cast<InvoicesTab *>(m_tabs->widget(1));
    if (tab)
        tab->openEditInvoiceDialog(invoiceId);
}

void MainWindow::onAddProduct()
{
    ProductDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted)
        return;

    Product p = dlg.product();
    p.id = -1;

    ProductRepository repo(m_db.db());
    int id = -1;
    if (!repo.create(p, &id)) {
        QMessageBox::warning(this, tr("Save failed"), repo.lastError());
        return;
    }
    m_model->refreshAll();
    m_view->selectRow(0);
}

void MainWindow::onEditProduct()
{
    const auto idx = m_view->currentIndex();
    if (!idx.isValid())
        return;

    ProductDialog dlg(this);
    dlg.setProduct(m_model->productAt(idx.row()));
    if (dlg.exec() != QDialog::Accepted)
        return;

    Product p = dlg.product();
    ProductRepository repo(m_db.db());
    if (!repo.update(p)) {
        QMessageBox::warning(this, tr("Save failed"), repo.lastError());
        return;
    }
    m_model->refreshOne(p.id);
}

void MainWindow::onDeleteProduct()
{
    const auto idx = m_view->currentIndex();
    if (!idx.isValid())
        return;

    const Product p = m_model->productAt(idx.row());
    auto answer = QMessageBox::question(
        this, tr("Delete product"),
        tr("Delete \"%1\"? This cannot be undone.").arg(p.name));
    if (answer != QMessageBox::Yes)
        return;

    ProductRepository repo(m_db.db());
    if (!repo.remove(p.id)) {
        QMessageBox::warning(this, tr("Delete failed"), repo.lastError());
        return;
    }
    m_model->refreshAll();
}

void MainWindow::onReceiveStock()
{
    const auto idx = m_view->currentIndex();
    if (!idx.isValid())
        return;
    adjustStockDialog(m_model->productAt(idx.row()).stock, tr("Receive Stock"));
}

void MainWindow::onAdjustStock()
{
    const auto idx = m_view->currentIndex();
    if (!idx.isValid())
        return;
    const Product p = m_model->productAt(idx.row());
    adjustStockDialog(p.stock, tr("Adjust Stock — %1").arg(p.name));
}

void MainWindow::adjustStockDialog(int current, const QString &title)
{
    const auto idx = m_view->currentIndex();
    const Product p = m_model->productAt(idx.row());

    bool ok = false;
    const int delta = QInputDialog::getInt(
        this, title,
        tr("Current stock: %1\nEnter adjustment (+ receive / − remove):")
            .arg(current),
        0, -1000000, 1000000, 1, &ok);

    if (!ok || delta == 0)
        return;

    ProductRepository repo(m_db.db());
    if (!repo.addStock(p.id, delta)) {
        QMessageBox::warning(this, tr("Adjust failed"), repo.lastError());
        return;
    }
    m_model->refreshOne(p.id);
}

void MainWindow::onSearchChanged(const QString &text)
{
    ProductFilter f;
    f.query = text.trimmed();
    f.lowStockOnly = m_lowStockCheck->isChecked();
    m_model->setFilter(f);
}

void MainWindow::onToggleLowStock(bool checked)
{
    ProductFilter f;
    f.lowStockOnly = checked;
    m_model->setFilter(f);
}
