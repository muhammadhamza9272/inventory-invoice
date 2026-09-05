#pragma once

#include "model/invoice.h"

#include <QSqlDatabase>

#include <optional>
#include <vector>

namespace inv {

// Persists invoices (header + line items) atomically in a transaction.
class InvoiceRepository
{
public:
    explicit InvoiceRepository(QSqlDatabase db) : m_db(std::move(db)) {}

    // Inserts a new invoice with all its items. Generates number if empty.
    bool create(Invoice &invoice);
    bool update(const Invoice &invoice);
    bool remove(int id);

    std::optional<Invoice> findById(int id) const;

    // Header rows only (no items) — for list views.
    struct Summary {
        int id = 0;
        int customerId = 0;
        QString customerName;
        QString number;
        QString issueDate;
        double total = 0.0;
    };
    std::vector<Summary> listSummaries() const;

    // ---- Reporting ----

    struct SalesSummary {
        int invoiceCount = 0;
        double revenue = 0.0;   // sum of grand totals
        double tax = 0.0;       // sum of tax
    };
    // Totals for invoices issued within [from, to] inclusive (yyyy-MM-dd).
    SalesSummary salesSummary(const QString &from, const QString &to) const;

    struct CustomerSales {
        int customerId = 0;
        QString customerName;
        double revenue = 0.0;
    };
    // Revenue grouped by customer within the same date window (sorted desc).
    std::vector<CustomerSales> salesByCustomer(const QString &from, const QString &to) const;

    // Issues the next sequential invoice number, e.g. INV-0001.
    QString nextNumber() const;

    QString lastError() const { return m_lastError; }

private:
    bool insertItems(int invoiceId, const std::vector<LineItem> &items);

    QSqlDatabase m_db;
    mutable QString m_lastError;
};

} // namespace inv
