#pragma once

#include "model/invoice.h"

#include <QString>

namespace inv {

// Renders an invoice to a PDF file. Pure QPainter drawing on QPdfWriter —
// no HTML templates, deterministic layout, easy to extend.
class InvoicePdfExporter
{
public:
    // Lays out the invoice on standard A4 portrait pages.
    bool exportTo(const Invoice &invoice, const Customer &customer, const QString &filePath,
                  QString *err = nullptr) const;

    // Assembles the suggested file name, e.g. "INV-0003.pdf".
    static QString suggestedFileName(const Invoice &invoice);
};

} // namespace inv