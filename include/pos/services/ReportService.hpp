// ReportService.hpp                                   Owner: Person 4
// Computes 6 tabular reports for manager oversight.

#ifndef POS_REPORT_SERVICE_HPP
#define POS_REPORT_SERVICE_HPP

#include <string>
#include <vector>

#include "pos/domain/Bill.hpp"
#include "pos/domain/Order.hpp"
#include "pos/domain/Product.hpp"
#include "pos/services/ReportTable.hpp"
#include "pos/storage/IBillRepository.hpp"
#include "pos/storage/IOrderDetailRepository.hpp"
#include "pos/storage/IOrderRepository.hpp"
#include "pos/storage/IProductRepository.hpp"

class ReportService {
public:
    ReportService(IOrderRepository& orders,
        IOrderDetailRepository& orderDetails,
        IBillRepository& bills,
        IProductRepository& products);

    // 1. Incomplete / pending orders
    ReportTable incompleteOrders() const;

    // 2. Completed / paid orders
    ReportTable completedOrders() const;

    // 3. Cash sales by date period [startDate, endDate]
    ReportTable cashSalesByPeriod(const std::string& startDate, const std::string& endDate) const;

    // 4. All sales with discount breakdown
    ReportTable allSalesWithDiscounts() const;

    // 5. Top 10 most sold items
    ReportTable top10MostSoldItems() const;

    // 6. Top 10 items by remaining stock
    ReportTable top10ByRemainingStock() const;

private:
    IOrderRepository& m_orders;
    IOrderDetailRepository& m_orderDetails;
    IBillRepository& m_bills;
    IProductRepository& m_products;
};

#endif // POS_REPORT_SERVICE_HPP