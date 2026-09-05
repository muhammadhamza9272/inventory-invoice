#include "invoice_pdf_exporter.h"

#include <QLocale>
#include <QPagedPaintDevice>
#include <QPainter>
#include <QPdfWriter>
#include <QPen>
#include <QRect>
#include <QStringList>

namespace inv {

namespace {
constexpr int kMargin = 40;          // points
constexpr int kPageW = 595;          // A4 portrait, points
constexpr int kContentW = kPageW - 2 * kMargin;
constexpr int kHeaderY = 64;
constexpr int kTableTop = 260;
constexpr int kRowH = 22;
constexpr int kFontH = 11;
constexpr int kColQty = kMargin + 400;
constexpr int kColPrice = kColQty + 50;
constexpr int kColTax = kColPrice + 50;
constexpr int kColTotal = kColTax + 60;
constexpr int kRightX = kPageW - kMargin;

// drawText on an anchored right edge without manual rectangle math.
void drawRight(QPainter &p, int x1, int x2, int y, const QString &text)
{
    p.drawText(QRect(x1, y - 15, x2 - x1, 20), Qt::AlignRight | Qt::AlignVCenter, text);
}
} // namespace

QString InvoicePdfExporter::suggestedFileName(const Invoice &invoice)
{
    return invoice.number + ".pdf";
}

bool InvoicePdfExporter::exportTo(const Invoice &invoice, const Customer &customer,
                                  const QString &filePath, QString *err) const
{
    QPdfWriter writer(filePath);
    writer.setPageSize(QPageSize(QPageSize::A4));
    writer.setResolution(72); // points

    QPainter painter(&writer);
    if (!painter.isActive()) {
        if (err)
            *err = QStringLiteral("Could not open file for writing");
        return false;
    }

    const auto setFont = [&painter](int size, bool bold = false) {
        QFont f;
        f.setPointSize(size);
        f.setBold(bold);
        painter.setFont(f);
    };

    // Header / branding
    setFont(20, true);
    painter.drawText(kMargin, kHeaderY, QStringLiteral("INVOICE"));

    setFont(kFontH);
    drawRight(painter, kMargin, kRightX, kHeaderY, invoice.number);

    setFont(10);
    drawRight(painter, kMargin, kRightX, kHeaderY + 18,
              QStringLiteral("Issued: %1").arg(invoice.issueDate));
    drawRight(painter, kMargin, kRightX, kHeaderY + 34,
              QStringLiteral("Due: %1").arg(invoice.dueDate));

    // Sold-to block
    setFont(10, true);
    painter.drawText(kMargin, kHeaderY + 50, QStringLiteral("SOLD TO"));
    setFont(kFontH);
    painter.drawText(kMargin, kHeaderY + 66, customer.name);
    int addrY = kHeaderY + 80;
    if (!customer.billingAddress.isEmpty()) {
        const QStringList lines = customer.billingAddress.split(QLatin1Char('\n'));
        for (const auto &line : lines) {
            painter.drawText(kMargin, addrY, line);
            addrY += 13;
        }
    }

    // Items table
    const int tableTop = kTableTop;
    painter.drawLine(kMargin, tableTop - 6, kRightX, tableTop - 6);

    setFont(kFontH, true);
    painter.drawText(kMargin, tableTop, QStringLiteral("Description"));
    drawRight(painter, kColQty, kColPrice, tableTop, QStringLiteral("Qty"));
    drawRight(painter, kColPrice, kColTax, tableTop, QStringLiteral("Unit"));
    drawRight(painter, kColTax, kColTotal, tableTop, QStringLiteral("Tax %"));
    drawRight(painter, kColTotal, kRightX, tableTop, QStringLiteral("Total"));
    painter.drawLine(kMargin, tableTop + 4, kRightX, tableTop + 4);

    setFont(kFontH);
    int y = tableTop + kRowH;
    for (const auto &item : invoice.items) {
        if (y > 700) { // page break: new page
            writer.newPage();
            y = 64;
            setFont(kFontH);
        }
        painter.drawText(kMargin, y, item.description);
        drawRight(painter, kColQty, kColPrice, y, QString::number(item.quantity, 'g', 4));
        drawRight(painter, kColPrice, kColTax, y,
                  QLocale().toCurrencyString(item.unitPrice));
        drawRight(painter, kColTax, kColTotal, y, QString::number(item.taxRate, 'g', 3));
        drawRight(painter, kColTotal, kRightX, y,
                  QLocale().toCurrencyString(item.lineTotalWithTax()));
        y += kRowH;
    }

    // Totals
    y += 12;
    setFont(kFontH);
    painter.drawText(kMargin, y, QStringLiteral("Subtotal"));
    drawRight(painter, kColTotal, kRightX, y,
              QLocale().toCurrencyString(invoice.subtotal()));
    y += kRowH;
    painter.drawText(kMargin, y, QStringLiteral("Tax"));
    drawRight(painter, kColTotal, kRightX, y,
              QLocale().toCurrencyString(invoice.totalTax()));
    y += kRowH;
    setFont(13, true);
    painter.drawText(kMargin, y, QStringLiteral("TOTAL"));
    drawRight(painter, kColTotal, kRightX, y,
              QLocale().toCurrencyString(invoice.total()));

    if (!invoice.note.isEmpty()) {
        y += kRowH + 8;
        setFont(9);
        painter.drawText(kMargin, y, QStringLiteral("Note: %1").arg(invoice.note));
    }

    painter.end();
    return true;
}

} // namespace inv