# Inventory & Invoice

A cross-platform desktop inventory and invoicing application for small
businesses. Built with **Qt 6 (Widgets)** and **SQLite** — single-file
database, no server required.

> This is the flagship portfolio project of a C++/Qt developer. It is
> engineered to be production-quality: clean MVC-style layering, atomic DB
> writes, unit tests, and a CMake build that works identically on
> **Windows, macOS, and Linux**.

## Features

### Inventory
- Products with SKU, unit price, stock, and reorder level
- Add / edit / delete products
- Search by name or SKU (live filtering)
- Low-stock highlighting and a "low stock only" filter
- Receive stock / adjust stock

### Invoicing
- Customers (name, email, phone, billing address)
- Invoice editor with an arbitrary number of line items
- Per-line quantity, unit price, and tax rate (percent)
- Live subtotal / tax / grand total
- Auto invoice numbering (`INV-0001`, …)
- Edit or delete saved invoices
- **Export to PDF** (A4, canvas-rendered, multi-page)

### Reporting
- Sales summary for any date range: invoice count, revenue, tax
- Revenue broken down by customer
- Low-stock products list

## Screenshots

_(TBD — add after packaging: main window inventory tab, invoice editor, PDF
output, reports tab.)_

## Build

Requirements: Qt 6.x (Widgets, Sql), CMake ≥ 3.16, a C++17 compiler.

```sh
cmake -S . -B build [ -DCMAKE_PREFIX_PATH=/path/to/qt/lib/cmake ]
cmake --build build
ctest --test-dir build     # run the test suite
./build/inventory_invoice
```

> On macOS with Homebrew Qt, the prefix path is
> `/opt/homebrew/opt/qtbase/lib/cmake`.

The application stores its database at the platform's standard application
data location (e.g. `~/Library/Application Support/inventory-invoice/` on
macOS, `%APPDATA%` on Windows).

## Architecture

```
src/
  app/main.cpp            entry point; opens the database, shows MainWindow
  model/                  plain data structures (Product, Invoice, LineItem,
                          Customer) + calculation logic
  data/                   persistence layer; all SQL isolated here
    database.h/.cpp       SQLite schema (DDL) and connection
    product_repository    product CRUD / search / stock
    customer_repository   customer CRUD / search
    invoice_repository    transactional invoice save, summaries, reporting
  ui/                     Qt Widgets: table models, dialogs, tabs, PDF export
test/                     Qt Test suite (data layer + math)
```

Design notes:

- **MVC separation**: models hold data and math; repositories own SQL; the UI
  layer only talks to repositories. The SQLite connection is injected so tests
  can use an in-memory database.
- **Atomic invoices**: header + line items are inserted/updated in a single
  transaction; deleting an invoice cascades to its items.
- **Prepared statements** everywhere (no string-built SQL for data).
- **Portable build**: no hardcoded Qt paths; `find_package(Qt6)` is all that's
  required.

## Testing

The test suite covers invoice math, product/customer CRUD, the invoice
lifecycle (create → reload → summaries → update → delete), number
generation, and reporting aggregates.

```sh
ctest --test-dir build --output-on-failure
```

## Roadmap

- [x] Core inventory + invoicing + PDF export + reports
- [ ] Packaged installers (macOS `.app`, Windows, `.deb`)
- [ ] Sales report charts
- [ ] Backup / restore database
- [ ] Multiple currencies / VAT presets

## License

MIT (TBD — confirm before publishing).

_Project by a freelance C++/Qt developer. Built to demonstrate idiomatic,
production-ready Qt engineering._