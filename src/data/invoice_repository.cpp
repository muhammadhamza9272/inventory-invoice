#include "invoice_repository.h"

#include <QSqlError>
#include <QSqlQuery>

#include <QStringLiteral>

namespace inv {

namespace {
QString nonNull(const QString &s) { return s.isEmpty() ? QStringLiteral("") : s; }
}

QString InvoiceRepository::nextNumber() const
{
    QSqlQuery q(m_db);
    q.exec("SELECT COUNT(*) FROM invoices");
    if (!q.next())
        return QStringLiteral("INV-0001");
    const int count = q.value(0).toInt();
    return QStringLiteral("INV-%1").arg(count + 1, 4, 10, QLatin1Char('0'));
}

bool InvoiceRepository::create(Invoice &invoice)
{
    if (invoice.number.isEmpty())
        invoice.number = nextNumber();

    if (!m_db.transaction()) {
        m_lastError = m_db.lastError().text();
        return false;
    }

    QSqlQuery q(m_db);
    q.prepare("INSERT INTO invoices (customer_id, number, issue_date, due_date, note) "
              "VALUES (?, ?, ?, ?, ?)");
    q.addBindValue(invoice.customerId);
    q.addBindValue(nonNull(invoice.number));
    q.addBindValue(nonNull(invoice.issueDate));
    q.addBindValue(nonNull(invoice.dueDate));
    q.addBindValue(nonNull(invoice.note));

    if (!q.exec()) {
        m_lastError = q.lastError().text();
        m_db.rollback();
        return false;
    }

    invoice.id = q.lastInsertId().toInt();
    if (!insertItems(invoice.id, invoice.items)) {
        m_db.rollback();
        return false;
    }

    if (!m_db.commit()) {
        m_lastError = m_db.lastError().text();
        m_db.rollback();
        return false;
    }
    return true;
}

bool InvoiceRepository::update(const Invoice &invoice)
{
    if (!m_db.transaction()) {
        m_lastError = m_db.lastError().text();
        return false;
    }

    QSqlQuery q(m_db);
    q.prepare("UPDATE invoices SET customer_id=?, number=?, issue_date=?, due_date=?, "
              "note=? WHERE id=?");
    q.addBindValue(invoice.customerId);
    q.addBindValue(nonNull(invoice.number));
    q.addBindValue(nonNull(invoice.issueDate));
    q.addBindValue(nonNull(invoice.dueDate));
    q.addBindValue(nonNull(invoice.note));
    q.addBindValue(invoice.id);
    if (!q.exec()) {
        m_lastError = q.lastError().text();
        m_db.rollback();
        return false;
    }

    QSqlQuery del(m_db);
    if (!del.exec("DELETE FROM invoice_items WHERE invoice_id=" +
                  QString::number(invoice.id))) {
        m_lastError = del.lastError().text();
        m_db.rollback();
        return false;
    }

    if (!insertItems(invoice.id, invoice.items)) {
        m_db.rollback();
        return false;
    }

    if (!m_db.commit()) {
        m_lastError = m_db.lastError().text();
        m_db.rollback();
        return false;
    }
    return true;
}

bool InvoiceRepository::remove(int id)
{
    QSqlQuery q(m_db);
    q.prepare("DELETE FROM invoices WHERE id=?");
    q.addBindValue(id);
    if (!q.exec()) {
        m_lastError = q.lastError().text();
        return false;
    }
    return true;
}

bool InvoiceRepository::insertItems(int invoiceId, const std::vector<LineItem> &items)
{
    for (const auto &item : items) {
        QSqlQuery q(m_db);
        q.prepare("INSERT INTO invoice_items (invoice_id, description, quantity, "
                  "unit_price, tax_rate) VALUES (?, ?, ?, ?, ?)");
        q.addBindValue(invoiceId);
        q.addBindValue(nonNull(item.description));
        q.addBindValue(item.quantity);
        q.addBindValue(item.unitPrice);
        q.addBindValue(item.taxRate);
        if (!q.exec()) {
            m_lastError = q.lastError().text();
            return false;
        }
    }
    return true;
}

std::optional<Invoice> InvoiceRepository::findById(int id) const
{
    Invoice inv;
    inv.id = id;

    QSqlQuery qh(m_db);
    qh.prepare("SELECT customer_id, number, issue_date, due_date, note FROM invoices WHERE id=?");
    qh.addBindValue(id);
    if (!qh.exec() || !qh.next())
        return std::nullopt;
    inv.customerId = qh.value(0).toInt();
    inv.number = qh.value(1).toString();
    inv.issueDate = qh.value(2).toString();
    inv.dueDate = qh.value(3).toString();
    inv.note = qh.value(4).toString();

    QSqlQuery qi(m_db);
    qi.prepare("SELECT id, description, quantity, unit_price, tax_rate "
               "FROM invoice_items WHERE invoice_id=? ORDER BY id");
    qi.addBindValue(id);
    if (qi.exec()) {
        while (qi.next()) {
            LineItem item;
            item.id = qi.value(0).toInt();
            item.description = qi.value(1).toString();
            item.quantity = qi.value(2).toDouble();
            item.unitPrice = qi.value(3).toDouble();
            item.taxRate = qi.value(4).toDouble();
            inv.items.push_back(item);
        }
    }
    return inv;
}

std::vector<InvoiceRepository::Summary> InvoiceRepository::listSummaries() const
{
    QSqlQuery q(m_db);
    q.exec(
        "SELECT i.id, i.customer_id, c.name, i.number, i.issue_date, "
        "       COALESCE(SUM(ii.quantity * ii.unit_price * (1 + ii.tax_rate / 100.0)), 0) "
        "FROM invoices i "
        "LEFT JOIN customers c ON c.id = i.customer_id "
        "LEFT JOIN invoice_items ii ON ii.invoice_id = i.id "
        "GROUP BY i.id "
        "ORDER BY i.id DESC");

    std::vector<Summary> out;
    while (q.next()) {
        Summary s;
        s.id = q.value(0).toInt();
        s.customerId = q.value(1).toInt();
        s.customerName = q.value(2).toString();
        s.number = q.value(3).toString();
        s.issueDate = q.value(4).toString();
        s.total = q.value(5).toDouble();
        out.push_back(s);
    }
    return out;
}

InvoiceRepository::SalesSummary InvoiceRepository::salesSummary(const QString &from,
                                                                const QString &to) const
{
    SalesSummary out;
    QSqlQuery q(m_db);
    q.prepare(
        "SELECT COUNT(DISTINCT i.id), "
        "       COALESCE(SUM(ii.quantity * ii.unit_price * (1 + ii.tax_rate / 100.0)), 0), "
        "       COALESCE(SUM(ii.quantity * ii.unit_price * (ii.tax_rate / 100.0)), 0) "
        "FROM invoices i "
        "LEFT JOIN invoice_items ii ON ii.invoice_id = i.id "
        "WHERE i.issue_date BETWEEN ? AND ?");
    q.addBindValue(from);
    q.addBindValue(to);
    if (q.exec() && q.next()) {
        out.invoiceCount = q.value(0).toInt();
        out.revenue = q.value(1).toDouble();
        out.tax = q.value(2).toDouble();
    }
    return out;
}

std::vector<InvoiceRepository::CustomerSales>
InvoiceRepository::salesByCustomer(const QString &from, const QString &to) const
{
    QSqlQuery q(m_db);
    q.prepare(
        "SELECT c.id, c.name, "
        "       COALESCE(SUM(ii.quantity * ii.unit_price * (1 + ii.tax_rate / 100.0)), 0) "
        "FROM invoices i "
        "JOIN customers c ON c.id = i.customer_id "
        "LEFT JOIN invoice_items ii ON ii.invoice_id = i.id "
        "WHERE i.issue_date BETWEEN ? AND ? "
        "GROUP BY c.id, c.name "
        "ORDER BY 3 DESC");
    q.addBindValue(from);
    q.addBindValue(to);

    std::vector<CustomerSales> out;
    if (!q.exec())
        return out;
    while (q.next()) {
        CustomerSales s;
        s.customerId = q.value(0).toInt();
        s.customerName = q.value(1).toString();
        s.revenue = q.value(2).toDouble();
        out.push_back(s);
    }
    return out;
}

} // namespace inv
