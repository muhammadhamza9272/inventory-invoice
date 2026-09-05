#include "demo_seed.h"

#include "data/database.h"
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

    int initialTab = 0;
    const int tabIdx = args.indexOf(QStringLiteral("--tab"));
    if (tabIdx > 0 && tabIdx + 1 < args.size()) {
        bool ok = false;
        const int value = args.at(tabIdx + 1).toInt(&ok);
        if (ok)
            initialTab = value;
    }

    inv::MainWindow win(db, initialTab);
    win.show();

    // --new-invoice opens the New Invoice editor once the window is up
    // (used for taking the invoice-editor screenshot).
    if (args.contains(QStringLiteral("--new-invoice")))
        QTimer::singleShot(600, &win, &inv::MainWindow::openInvoiceEditorForNewInvoice);

    return app.exec();
}