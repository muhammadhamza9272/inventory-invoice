#include "reports_tab.h"

#include "data/database.h"
#include "data/invoice_repository.h"
#include "ui/product_table_model.h"
#include "ui/sales_by_customer_model.h"

#include <QAbstractItemView>
#include <QDate>
#include <QDateEdit>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLocale>
#include <QPushButton>
#include <QScrollArea>
#include <QTableView>
#include <QVBoxLayout>

namespace inv {

ReportsTab::ReportsTab(Database &db, QWidget *parent)
    : QWidget(parent), m_db(db)
{
    const QDate defaultFrom = QDate::currentDate().addMonths(-1);
    const QDate defaultTo = QDate::currentDate();

    m_from = new QDateEdit(defaultFrom, this);
    m_from->setCalendarPopup(true);
    m_from->setDisplayFormat("yyyy-MM-dd");
    m_to = new QDateEdit(defaultTo, this);
    m_to->setCalendarPopup(true);
    m_to->setDisplayFormat("yyyy-MM-dd");

    auto *rangeLabel = new QLabel(tr("Period:"), this);
    auto *refreshBtn = new QPushButton(tr("Refresh"), this);
    connect(refreshBtn, &QPushButton::clicked, this, &ReportsTab::refresh);

    auto *rangeRow = new QHBoxLayout;
    rangeRow->addWidget(rangeLabel);
    rangeRow->addWidget(m_from);
    rangeRow->addWidget(new QLabel(tr("to"), this));
    rangeRow->addWidget(m_to);
    rangeRow->addWidget(refreshBtn);
    rangeRow->addStretch();

    // ---- Summary labels ----
    m_invoicesLabel = new QLabel(this);
    m_revenueLabel = new QLabel(this);
    m_taxLabel = new QLabel(this);
    m_revenueLabel->setStyleSheet("font-weight: bold; font-size: 15px;");

    auto *summaryBox = new QGroupBox(tr("Sales Summary"), this);
    auto *summaryGrid = new QGridLayout(summaryBox);
    summaryGrid->addWidget(new QLabel(tr("Invoices:"), summaryBox), 0, 0);
    summaryGrid->addWidget(m_invoicesLabel, 0, 1);
    summaryGrid->addWidget(new QLabel(tr("Revenue:"), summaryBox), 1, 0);
    summaryGrid->addWidget(m_revenueLabel, 1, 1);
    summaryGrid->addWidget(new QLabel(tr("Tax:"), summaryBox), 2, 0);
    summaryGrid->addWidget(m_taxLabel, 2, 1);
    summaryGrid->setColumnStretch(1, 1);

    // ---- Sales by customer ----
    m_byCustomerModel = new SalesByCustomerModel(m_db.db(), this);
    m_byCustomerView = new QTableView(this);
    m_byCustomerView->setModel(m_byCustomerModel);
    m_byCustomerView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_byCustomerView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_byCustomerView->verticalHeader()->setVisible(false);
    m_byCustomerView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    auto *byCustomerTitle = new QLabel(tr("<b>Revenue by customer</b>"), this);

    // ---- Low stock ----
    m_lowStockModel = new ProductTableModel(m_db.db(), this);
    ProductFilter lowFilter;
    lowFilter.lowStockOnly = true;
    m_lowStockModel->setFilter(lowFilter);

    m_lowStockView = new QTableView(this);
    m_lowStockView->setModel(m_lowStockModel);
    m_lowStockView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_lowStockView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_lowStockView->verticalHeader()->setVisible(false);
    m_lowStockView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_lowStockView->setMinimumHeight(160);

    auto *lowStockTitle = new QLabel(tr("<b>Low stock products</b>"), this);

    // ---- Layout ----
    auto *content = new QWidget(this);
    auto *root = new QVBoxLayout(content);
    root->addLayout(rangeRow);
    root->addWidget(summaryBox);
    root->addWidget(byCustomerTitle);
    root->addWidget(m_byCustomerView, 2);
    root->addWidget(lowStockTitle);
    root->addWidget(m_lowStockView, 1);

    auto *scroll = new QScrollArea(this);
    scroll->setWidget(content);
    scroll->setWidgetResizable(true);

    auto *outer = new QVBoxLayout(this);
    outer->addWidget(scroll);

    refresh();
}

void ReportsTab::refresh()
{
    const QString from = m_from->date().toString("yyyy-MM-dd");
    const QString to = m_to->date().toString("yyyy-MM-dd");

    InvoiceRepository repo(m_db.db());
    const auto summary = repo.salesSummary(from, to);

    m_invoicesLabel->setText(QString::number(summary.invoiceCount));
    m_revenueLabel->setText(QLocale().toCurrencyString(summary.revenue));
    m_taxLabel->setText(QLocale().toCurrencyString(summary.tax));

    m_byCustomerModel->refresh(from, to);
    m_lowStockView->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
}

} // namespace inv