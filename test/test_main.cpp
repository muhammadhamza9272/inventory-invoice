#include "../src/data/customer_repository.h"
#include "../src/data/database.h"
#include "../src/data/invoice_repository.h"
#include "../src/data/product_repository.h"
#include "../src/model/invoice.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QtTest/QtTest>

using namespace inv;

class InventoryTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase()
    {
        // In-memory SQLite for isolated, fast tests.
        QVERIFY(m_db.open(":memory:"));
        m_products.reset(new ProductRepository(m_db.db()));
    }

    void invoiceMath_data()
    {
        QTest::addColumn<LineItem>("extra");
        // a single item of 2 x 100 @ 13%
        QTest::newRow("single") << LineItem{1, "Widget", 2.0, 100.0, 13.0};
    }

    void invoiceMath()
    {
        Invoice inv;
        inv.items.push_back(LineItem{1, "Widget", 2.0, 100.0, 13.0});
        QCOMPARE(inv.subtotal(), 200.0);
        QCOMPARE(inv.totalTax(), 26.0);
        QCOMPARE(inv.total(), 226.0);
    }

    void productCrud()
    {
        Product p;
        p.name = "HP Laser Printer";
        p.sku = "HPLAS-M1212";
        p.unitPrice = 125.5;
        p.stock = 4;
        p.reorderLevel = 2;

        int id = -1;
        QVERIFY(m_products->create(p, &id));
        QVERIFY(id > 0);

        auto found = m_products->findById(id);
        QVERIFY(found.has_value());
        QCOMPARE(found->name, p.name);
        QCOMPARE(found->stock, 4);

        // receive stock
        QVERIFY(m_products->addStock(id, 6));
        found = m_products->findById(id);
        QCOMPARE(found->stock, 10);

        // filter by low stock: stock 10 > reorder 2, so low-stock list empty
        ProductFilter f;
        f.lowStockOnly = true;
        QVERIFY(m_products->list(f).empty());

        // remove stock below reorder → now low stock
        QVERIFY(m_products->addStock(id, -9));
        f = ProductFilter{};
        f.lowStockOnly = true;
        auto low = m_products->list(f);
        QCOMPARE(low.size(), 1);
        QCOMPARE(low[0].stock, 1);

        QVERIFY(m_products->remove(id));
        QVERIFY(!m_products->findById(id).has_value());
    }

    void searchFilter()
    {
        m_products->create(Product{1, "Chair", "CHR-1", 50.0, 5, 0});
        m_products->create(Product{2, "Desk", "DSK-1", 200.0, 3, 0});
        ProductFilter f;
        f.query = "desk";
        auto rows = m_products->list(f);
        QCOMPARE(rows.size(), 1);
        QCOMPARE(rows[0].name, "Desk");
    }

    void customerCrud()
    {
        CustomerRepository repo(m_db.db());
        Customer c;
        c.name = "ACME Corp";
        c.email = "billing@acme.test";
        c.phone = "+1-555-0100";
        c.billingAddress = "1 Main St\nSpringfield";

        int id = -1;
        QVERIFY(repo.create(c, &id));
        QVERIFY(id > 0);

        auto found = repo.findById(id);
        QVERIFY(found.has_value());
        QCOMPARE(found->name, "ACME Corp");
        QCOMPARE(found->billingAddress, "1 Main St\nSpringfield");

        auto matches = repo.list("acme");
        QCOMPARE(matches.size(), 1);

        c.id = id;
        c.name = "ACME International";
        QVERIFY(repo.update(c));
        found = repo.findById(id);
        QCOMPARE(found->name, "ACME International");

        QVERIFY(repo.remove(id));
        QVERIFY(!repo.findById(id).has_value());
    }

    void invoiceLifecycle()
    {
        // Customer used by the invoice must exist for join in summaries.
        CustomerRepository custRepo(m_db.db());
        Customer c;
        c.name = "Alpha Buyer";
        int custId = -1;
        QVERIFY(custRepo.create(c, &custId));

        InvoiceRepository repo(m_db.db());
        Invoice inv;
        inv.customerId = custId;
        inv.issueDate = "2026-09-05";
        inv.dueDate = "2026-09-19";
        inv.note = "Thanks for your business";
        inv.items.push_back(LineItem{1, "Widget", 2.0, 100.0, 13.0});
        inv.items.push_back(LineItem{1, "Gadget", 1.0, 50.0, 0.0});

        QVERIFY(repo.create(inv));
        QVERIFY(inv.id >= 0);
        QVERIFY(!inv.number.isEmpty()); // auto-generated
        QCOMPARE(inv.total(), 276.0);   // 2*100*1.13 + 50

        auto loaded = repo.findById(inv.id);
        QVERIFY(loaded.has_value());
        QCOMPARE(loaded->items.size(), 2);
        QCOMPARE(loaded->number, inv.number);
        QCOMPARE(loaded->total(), 276.0);

        auto sums = repo.listSummaries();
        QVERIFY(sums.size() >= 1);
        const auto &mySum = *std::find_if(sums.begin(), sums.end(),
                                          [&](const auto &s) { return s.id == inv.id; });
        QCOMPARE(mySum.total, 276.0);
        QCOMPARE(mySum.customerName, "Alpha Buyer");

        inv.items.push_back(LineItem{1, "Extra", 1.0, 10.0, 5.0});
        QVERIFY(repo.update(inv));
        loaded = repo.findById(inv.id);
        QCOMPARE(loaded->items.size(), 3);
        QCOMPARE(loaded->total(), 276.0 + 10.5);

        QVERIFY(repo.remove(inv.id));
        QVERIFY(!repo.findById(inv.id).has_value());
    }

    void nextNumber()
    {
        InvoiceRepository repo(m_db.db());
        QVERIFY(repo.nextNumber().startsWith("INV-"));
    }

    void reporting()
    {
        // Two customers, two invoices at different dates.
        CustomerRepository custRepo(m_db.db());
        int a = -1, b = -1;
        Customer ca; ca.name = "Cust A";
        Customer cb; cb.name = "Cust B";
        QVERIFY(custRepo.create(ca, &a));
        QVERIFY(custRepo.create(cb, &b));

        InvoiceRepository repo(m_db.db());
        Invoice inv1;
        inv1.customerId = a;
        inv1.issueDate = "2026-09-01";
        inv1.items.push_back(LineItem{1, "X", 2.0, 100.0, 0.0}); // 200
        QVERIFY(repo.create(inv1));

        Invoice inv2;
        inv2.customerId = b;
        inv2.issueDate = "2026-09-10";
        inv2.items.push_back(LineItem{1, "Y", 1.0, 50.0, 10.0}); // 55
        QVERIFY(repo.create(inv2));

        invoiceInclusiveHelper(repo, inv1, inv2, "2026-09-01", "2026-09-10", 255.0, 2);

        // Window that only includes inv1.
        invoiceInclusiveHelper(repo, inv1, inv2, "2026-09-01", "2026-09-05", 200.0, 1);

        // Window with no invoices.
        const auto empty = repo.salesSummary("2020-01-01", "2020-01-31");
        QCOMPARE(empty.invoiceCount, 0);
        QCOMPARE(empty.revenue, 0.0);

        // By-customer breakdown: A=200, B=55 (desc order).
        const auto byCust = repo.salesByCustomer("2026-09-01", "2026-09-10");
        QCOMPARE(byCust.size(), 2);
        QCOMPARE(byCust[0].customerName, "Cust A");
        QCOMPARE(byCust[0].revenue, 200.0);
        QCOMPARE(byCust[1].customerName, "Cust B");
        QCOMPARE(byCust[1].revenue, 55.0);
    }

private:
    void invoiceInclusiveHelper(InvoiceRepository &repo, const Invoice &first,
                                const Invoice &second, const QString &from,
                                const QString &to, double expectedRevenue, int expectedCount)
    {
        const auto s = repo.salesSummary(from, to);
        QCOMPARE(s.invoiceCount, expectedCount);
        QCOMPARE(s.revenue, expectedRevenue);
        Q_UNUSED(first);
        Q_UNUSED(second);
    }

    Database m_db;
    std::unique_ptr<ProductRepository> m_products;
};

QTEST_MAIN(InventoryTest)
#include "test_main.moc"
