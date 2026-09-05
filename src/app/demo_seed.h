#pragma once

#include <QSqlDatabase>

namespace inv {

// Populates an empty database with sample products/customers/invoices so the
// app can be demoed or screenshotted. Appends to existing rows; overwrites
// nothing. No-op if the DB already has products.
void seedDemoData(QSqlDatabase db);

} // namespace inv