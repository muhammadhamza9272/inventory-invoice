#pragma once

#include <QString>

#include <optional>
#include <vector>

namespace inv {

// A single line on an invoice.
struct LineItem {
    int id{-1};
    QString description;
    double quantity{1.0};
    double unitPrice{0.0};
    double taxRate{0.0}; // percentage, e.g. 13.0 == 13%

    double lineTotal() const;
    double taxAmount() const;
    double lineTotalWithTax() const;
};

// A customer an invoice is issued to.
struct Customer {
    int id{-1};
    QString name;
    QString email;
    QString phone;
    QString billingAddress;
};

// An invoice (header + rows).
struct Invoice {
    int id{-1};
    int customerId{-1};
    QString number;          // human-readable invoice number
    QString issueDate;       // ISO yyyy-MM-dd
    QString dueDate;
    QString note;
    std::vector<LineItem> items;

    double subtotal() const;
    double totalTax() const;
    double total() const;
};

} // namespace inv
