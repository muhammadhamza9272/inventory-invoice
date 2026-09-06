#include "demo_seed.h"

#include "data/customer_repository.h"
#include "data/database.h"
#include "data/invoice_repository.h"
#include "ui/invoice_pdf_exporter.h"
#include "ui/main_window.h"

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QMessageBox>
#include <QStandardPaths>
#include <QTimer>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setApplicationName("inventory-invoice");
    QCoreApplication::setOrganizationName("inventory-invoice");
    QCoreApplication::setApplicationVersion("0.1.0");

    const auto dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDir);
    const QString dbPath = dataDir + "/inventory.db";

    inv::Database db;
    if (!db.open(dbPath)) {
        QMessageBox::critical(nullptr, "Inventory & Invoice",
                              QString("Could not open database:\n%1").arg(db.lastError()));
        return 1;
    }
    qInfo() << "Database:" << dbPath;

    const QStringList args = app.arguments();

    if (args.contains(QStringLiteral("--demo"))) {
        inv::seedDemoData(db.db());
        qInfo() << "Demo data seeded.";
    }

    // --demo-pdf <dir> exports the most recent invoice to a PDF and exits.
    // (Reuses the same exporter the UI uses; handy for demos / CI screenshots.)
    const int pdfIdx = args.indexOf(QStringLiteral("--demo-pdf"));
    if (pdfIdx > 0 && pdfIdx + 1 < args.size()) {
        inv::InvoiceRepository invRepo(db.db());
        const auto summaries = invRepo.listSummaries();
        if (summaries.empty()) {
            qCritical() << "No invoices found — run with --demo first.";
            return 1;
        }
        const auto invoice = invRepo.findById(summaries.back().id);
        if (!invoice) {
            qCritical() << "Could not load invoice for PDF demo.";
            return 1;
        }
        inv::CustomerRepository custRepo(db.db());
        const auto customer = custRepo.findById(invoice->customerId);
        if (!customer) {
            qCritical() << "Could not load customer for PDF demo.";
            return 1;
        }
        const QString outDir = args.at(pdfIdx + 1);
        QDir().mkpath(outDir);
        const QString pdfPath = QDir(outDir).filePath(inv::InvoicePdfExporter::suggestedFileName(*invoice));
        QString err;
        if (!inv::InvoicePdfExporter().exportTo(*invoice, *customer, pdfPath, &err)) {
            qCritical() << "PDF export failed:" << err;
            return 1;
        }
        qInfo() << "Exported demo PDF to" << pdfPath;
        return 0;
    }

    int initialTab = 0;
    const int tabIdx = args.indexOf(QStringLiteral("--tab"));
    if (tabIdx > 0 && tabIdx + 1 < args.size()) {
        bool ok = false;
        const int value = args.at(tabIdx + 1).toInt(&ok);
        if (ok)
            initialTab = value;
    }

    int editInvoiceId = -1;
    const int eiIdx = args.indexOf(QStringLiteral("--edit-invoice"));
    if (eiIdx > 0 && eiIdx + 1 < args.size()) {
        bool ok = false;
        const int value = args.at(eiIdx + 1).toInt(&ok);
        if (ok)
            editInvoiceId = value;
    }

    inv::MainWindow win(db, initialTab);
    win.show();

    // --new-invoice opens the New Invoice editor once the window is up
    // (used for taking the invoice-editor screenshot).
    if (args.contains(QStringLiteral("--new-invoice")))
        QTimer::singleShot(600, &win, &inv::MainWindow::openInvoiceEditorForNewInvoice);

    // --edit-invoice <id> opens a saved invoice for editing (line items shown).
    if (editInvoiceId > 0)
        QTimer::singleShot(600, &win, [&win, editInvoiceId] {
            win.openInvoiceEditorForInvoice(editInvoiceId);
        });

    return app.exec();
}