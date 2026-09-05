#pragma once

#include <QSqlDatabase>

namespace inv {

// Owns the SQLite database connection and schema. All SQL DDL lives here.
class Database
{
public:
    // Opens (creating if needed) the database at path. On failure returns
    // false and lastError() holds the message. Path may be ":memory:".
    bool open(const QString &path);

    QString lastError() const { return m_lastError; }
    QSqlDatabase db() const { return m_db; }

private:
    bool createSchema();

    QSqlDatabase m_db;
    QString m_lastError;
};

} // namespace inv
