#include "database.h"

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

namespace inv {

bool Database::open(const QString &path)
{
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(path);
    if (!m_db.open()) {
        m_lastError = m_db.lastError().text();
        return false;
    }
    return createSchema();
}

bool Database::createSchema()
{
    const auto exec = [this](const char *sql) {
        QSqlQuery q(m_db);
        if (!q.exec(QLatin1String(sql))) {
            m_lastError = q.lastError().text();
            return false;
        }
        return true;
    };

    if (!exec(R"(
        CREATE TABLE IF NOT EXISTS products (
            id           INTEGER PRIMARY KEY AUTOINCREMENT,
            name         TEXT    NOT NULL,
            sku          TEXT    NOT NULL DEFAULT '',
            unit_price   REAL    NOT NULL DEFAULT 0,
            stock        INTEGER NOT NULL DEFAULT 0,
            reorder_level INTEGER NOT NULL DEFAULT 0
        );
    )"))
        return false;

    if (!exec(R"(
        CREATE TABLE IF NOT EXISTS customers (
            id               INTEGER PRIMARY KEY AUTOINCREMENT,
            name             TEXT NOT NULL,
            email            TEXT NOT NULL DEFAULT '',
            phone            TEXT NOT NULL DEFAULT '',
            billing_address  TEXT NOT NULL DEFAULT ''
        );
    )"))
        return false;

    if (!exec(R"(
        CREATE TABLE IF NOT EXISTS invoices (
            id         INTEGER PRIMARY KEY AUTOINCREMENT,
            customer_id INTEGER NOT NULL,
            number     TEXT    NOT NULL,
            issue_date TEXT    NOT NULL,
            due_date   TEXT    NOT NULL DEFAULT '',
            note       TEXT    NOT NULL DEFAULT '',
            FOREIGN KEY (customer_id) REFERENCES customers(id)
        );
    )"))
        return false;

    if (!exec(R"(
        CREATE TABLE IF NOT EXISTS invoice_items (
            id          INTEGER PRIMARY KEY AUTOINCREMENT,
            invoice_id  INTEGER NOT NULL,
            description TEXT    NOT NULL,
            quantity    REAL    NOT NULL DEFAULT 1,
            unit_price  REAL    NOT NULL DEFAULT 0,
            tax_rate    REAL    NOT NULL DEFAULT 0,
            FOREIGN KEY (invoice_id) REFERENCES invoices(id) ON DELETE CASCADE
        );
    )"))
        return false;

    return true;
}

} // namespace inv
