#include "invoice.h"

namespace inv {

double LineItem::lineTotal() const
{
    return quantity * unitPrice;
}

double LineItem::taxAmount() const
{
    return lineTotal() * (taxRate / 100.0);
}

double LineItem::lineTotalWithTax() const
{
    return lineTotal() + taxAmount();
}

double Invoice::subtotal() const
{
    double sum = 0.0;
    for (const auto &item : items)
        sum += item.lineTotal();
    return sum;
}

double Invoice::totalTax() const
{
    double sum = 0.0;
    for (const auto &item : items)
        sum += item.taxAmount();
    return sum;
}

double Invoice::total() const
{
    double sum = 0.0;
    for (const auto &item : items)
        sum += item.lineTotalWithTax();
    return sum;
}

} // namespace inv
