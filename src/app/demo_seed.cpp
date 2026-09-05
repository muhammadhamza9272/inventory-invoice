#include "demo_seed.h"

#include "../data/customer_repository.h"
#include "../data/database.h"
#include "../data/invoice_repository.h"
#include "../data/product_repository.h"

#include <QDate>

namespace inv {

void seedDemoData(QSqlDatabase db)
{
    ProductRepository products(db);
    if (!products.list().empty())
        return; // already demo'd / in use

    const auto addProduct = [&](const char *name, const char *sku, double price,
                                int stock, int reorder) {
        Product p;
        p.name = QString::fromLatin1(name);
        p.sku = QString::fromLatin1(sku);
        p.unitPrice = price;
        p.stock = stock;
        p.reorderLevel = reorder;
        products.create(p);
    };

    addProduct("Office Chair", "CHR-100", 89.99, 14, 5);
    addProduct("Standing Desk", "DSK-200", 345.00, 6, 3);
    addProduct("LED Monitor 27\"", "MON-27", 219.50, 9, 4);
    addProduct("Mechanical Keyboard", "KEY-MK", 129.00, 2, 5);
    addProduct("USB-C Dock 8-in-1", "DOCK-8", 74.25, 0, 3);
    addProduct("Noise-Cancelling Headset", "HEA-NC", 189.00, 12, 4);

    CustomerRepository customers(db);
    int c1 = -1, c2 = -1, c3 = -1;
    const auto addCustomer = [&](const char *name, const char *email,
                                 const char *phone, const char *addr, int *outId) {
        Customer c;
        c.name = QString::fromLatin1(name);
        c.email = QString::fromLatin1(email);
        c.phone = QString::fromLatin1(phone);
        c.billingAddress = QString::fromLatin1(addr);
        customers.create(c, outId);
    };
    addCustomer("Brightworks Studio", "ap@brightworks.example", "+1 555 0142",
                "12 Market St\nNew York, NY", &c1);
    addCustomer("Nova Retail Ltd", "office@novaretail.example", "+44 20 7946 0008",
                "5 Kings Road\nLondon", &c2);
    addCustomer("Helios Labs", "procurement@helioslabs.example", "+49 30 123456",
                "101 Hauptstrasse\nBerlin", &c3);

    InvoiceRepository invoices(db);
    const auto makeInvoice = [&](int customerId, int daysAgo, const char *note = "") {
        Invoice inv;
        inv.customerId = customerId;
        inv.issueDate = QDate::currentDate().addDays(-daysAgo).toString("yyyy-MM-dd");
        inv.dueDate = QDate::currentDate().addDays(14 - daysAgo).toString("yyyy-MM-dd");
        inv.note = QString::fromLatin1(note);
        return inv;
    };

    Invoice a = makeInvoice(c1, 20, "Thank you for your order");
    a.items.push_back({1, "LED Monitor 27\"", 2.0, 219.50, 13.0});
    a.items.push_back({1, "Office Chair", 1.0, 89.99, 13.0});
    invoices.create(a);

    Invoice b = makeInvoice(c2, 12);
    b.items.push_back({1, "Standing Desk", 3.0, 345.00, 20.0});
    b.items.push_back({1, "Noise-Cancelling Headset", 4.0, 189.00, 20.0});
    invoices.create(b);

    Invoice c = makeInvoice(c3, 5, "Net 30");
    c.items.push_back({1, "USB-C Dock 8-in-1", 8.0, 74.25, 19.0});
    c.items.push_back({1, "Mechanical Keyboard", 2.0, 129.00, 19.0});
    invoices.create(c);
}

} // namespace inv