// ReportService.cpp                                   Owner: Person 4
// Computes 6 tabular reports for manager oversight.

#include "pos/services/ReportService.hpp"
#include "pos/common/DateUtils.hpp"

#include <algorithm>
#include <iomanip>
#include <map>
#include <sstream>

namespace {

    std::string formatQty(double val) {
        std::ostringstream oss;
        if (val == static_cast<long long>(val)) {
            oss << static_cast<long long>(val);
        }
        else {
            oss << std::fixed << std::setprecision(2) << val;
        }
        return oss.str();
    }

} // namespace

ReportService::ReportService(IOrderRepository& orders,
    IOrderDetailRepository& orderDetails,
    IBillRepository& bills,
    IProductRepository& products)
    : m_orders(orders),
    m_orderDetails(orderDetails),
    m_bills(bills),
    m_products(products) {
}

ReportTable ReportService::incompleteOrders() const {
    ReportTable table;
    table.title = "Incomplete / Pending Orders";
    table.headers = { "Order ID", "Customer Name", "Date", "Status", "Total Amount" };

    auto allOrders = m_orders.all();
    Money totalSum;

    for (const auto& ord : allOrders) {
        if (ord.status != OrderStatus::Paid && ord.status != OrderStatus::Archived) {
            table.rows.push_back({
                ord.id,
                ord.customerName,
                ord.date,
                pos::toString(ord.status),
                ord.payment.toString()
                });
            totalSum += ord.payment;
        }
    }

    table.totalsRow = {
        "Total",
        "",
        "",
        std::to_string(table.rows.size()) + " orders",
        totalSum.toString()
    };

    return table;
}

ReportTable ReportService::completedOrders() const {
    ReportTable table;
    table.title = "Completed (Paid) Orders";
    table.headers = { "Order ID", "Customer Name", "Date", "Status", "Total Amount" };

    auto allOrders = m_orders.all();
    Money totalSum;

    for (const auto& ord : allOrders) {
        if (ord.status == OrderStatus::Paid) {
            table.rows.push_back({
                ord.id,
                ord.customerName,
                ord.date,
                pos::toString(ord.status),
                ord.payment.toString()
                });
            totalSum += ord.payment;
        }
    }

    table.totalsRow = {
        "Total",
        "",
        "",
        std::to_string(table.rows.size()) + " orders",
        totalSum.toString()
    };

    return table;
}

ReportTable ReportService::cashSalesByPeriod(const std::string& startDate, const std::string& endDate) const {
    ReportTable table;
    table.title = "Cash Sales (" + startDate + " to " + endDate + ")";
    table.headers = { "Bill ID", "Order ID", "Date", "Amount Received", "Total Payable", "Balance Returned" };

    auto allBills = m_bills.all();
    Money sumReceived;
    Money sumPayable;
    Money sumReturned;

    for (const auto& bill : allBills) {
        if (pos::DateUtils::isDateInRange(bill.date, startDate, endDate)) {
            table.rows.push_back({
                bill.id,
                bill.orderId,
                bill.date,
                bill.amountReceived.toString(),
                bill.totalPayable.toString(),
                bill.balanceReturned.toString()
                });
            sumReceived += bill.amountReceived;
            sumPayable += bill.totalPayable;
            sumReturned += bill.balanceReturned;
        }
    }

    table.totalsRow = {
        "Total",
        "",
        std::to_string(table.rows.size()) + " bills",
        sumReceived.toString(),
        sumPayable.toString(),
        sumReturned.toString()
    };

    return table;
}

ReportTable ReportService::allSalesWithDiscounts() const {
    ReportTable table;
    table.title = "All Sales with Discounts";
    table.headers = { "Bill ID", "Order ID", "Customer", "Date", "Prod Disc", "Season Disc", "Spec Disc", "Total Payable" };

    auto allBills = m_bills.all();
    Money sumProdDisc;
    Money sumSeasonDisc;
    Money sumSpecDisc;
    Money sumPayable;

    for (const auto& bill : allBills) {
        table.rows.push_back({
            bill.id,
            bill.orderId,
            bill.customerName,
            bill.date,
            bill.totalDiscount.toString(),
            bill.seasonalDiscount.toString(),
            bill.specialDiscount.toString(),
            bill.totalPayable.toString()
            });
        sumProdDisc += bill.totalDiscount;
        sumSeasonDisc += bill.seasonalDiscount;
        sumSpecDisc += bill.specialDiscount;
        sumPayable += bill.totalPayable;
    }

    table.totalsRow = {
        "Total",
        "",
        "",
        std::to_string(table.rows.size()) + " bills",
        sumProdDisc.toString(),
        sumSeasonDisc.toString(),
        sumSpecDisc.toString(),
        sumPayable.toString()
    };

    return table;
}

ReportTable ReportService::top10MostSoldItems() const {
    ReportTable table;
    table.title = "Top 10 Most Sold Items";
    table.headers = { "Rank", "Acronym", "Product Name", "Quantity Sold", "Remaining Stock" };

    auto allEntries = m_orderDetails.all();
    std::map<std::string, double> soldByBarcode;
    std::map<std::string, std::string> acronymByBarcode;

    for (const auto& entry : allEntries) {
        soldByBarcode[entry.line.barcode] += entry.line.quantity;
        acronymByBarcode[entry.line.barcode] = entry.line.acronym;
    }

    struct ItemSold {
        std::string barcode;
        std::string acronym;
        double qtySold;
    };

    std::vector<ItemSold> sorted;
    sorted.reserve(soldByBarcode.size());
    for (const auto& kv : soldByBarcode) {
        sorted.push_back({ kv.first, acronymByBarcode[kv.first], kv.second });
    }

    std::sort(sorted.begin(), sorted.end(), [](const ItemSold& a, const ItemSold& b) {
        return a.qtySold > b.qtySold;
        });

    size_t count = std::min(size_t(10), sorted.size());
    for (size_t i = 0; i < count; ++i) {
        const auto& item = sorted[i];
        std::string acronym = item.acronym;
        std::string name = "Unknown";
        std::string remaining = "N/A";

        auto optProduct = m_products.findByBarcode(item.barcode);
        if (optProduct.has_value()) {
            if (!optProduct->acronym.empty()) {
                acronym = optProduct->acronym;
            }
            name = optProduct->name;
            remaining = formatQty(optProduct->availableQty);
        }

        table.rows.push_back({
            std::to_string(i + 1),
            acronym,
            name,
            formatQty(item.qtySold),
            remaining
            });
    }

    return table;
}

ReportTable ReportService::top10ByRemainingStock() const {
    ReportTable table;
    table.title = "Top 10 Items by Remaining Stock";
    table.headers = { "Rank", "Barcode", "Acronym", "Product Name", "Remaining Stock" };

    auto allProducts = m_products.all();
    std::sort(allProducts.begin(), allProducts.end(), [](const Product& a, const Product& b) {
        return a.availableQty > b.availableQty;
        });

    size_t count = std::min(size_t(10), allProducts.size());
    for (size_t i = 0; i < count; ++i) {
        const auto& p = allProducts[i];
        table.rows.push_back({
            std::to_string(i + 1),
            p.barcode,
            p.acronym,
            p.name,
            formatQty(p.availableQty)
            });
    }

    return table;
}