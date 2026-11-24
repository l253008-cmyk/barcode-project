// OrderService.hpp                                    Owner: Person 5
// High-level service managing order draft lifecycle, hold/resume, and order editing.

#ifndef POS_ORDERSERVICE_HPP
#define POS_ORDERSERVICE_HPP

#include <string>
#include <vector>

#include "pos/common/IClock.hpp"
#include "pos/domain/Order.hpp"
#include "pos/services/IdGenerator.hpp"
#include "pos/services/ProductService.hpp"
#include "pos/storage/IOrderDetailRepository.hpp"
#include "pos/storage/IOrderRepository.hpp"
#include "pos/storage/IProductRepository.hpp"

class OrderService {
public:
    OrderService(IProductRepository& products,
                 ProductService& productService,
                 IOrderRepository& orders,
                 IOrderDetailRepository& orderDetails,
                 IdGenerator& idGenerator,
                 const IClock& clock);

    // Starts a new empty order (an unheld draft is discarded)
    void start();

    // Adds a product by barcode. If already in draft, increments quantity.
    // Stock check validates the combined quantity. Default qty = 1.0.
    void addItem(const std::string& barcode, double quantity = 1.0);

    // Removes a product line from the draft
    void removeItem(const std::string& barcode);

    // Changes the quantity of an existing line item in the order
    void updateItemQuantity(const std::string& barcode, double newQuantity);

    // Puts active draft on hold into Order.txt/OrderDetail.txt and starts a new order
    void hold();
    void holdCurrentOrder() { hold(); } // Backward compatibility alias

    // Returns all currently held orders
    std::vector<Order> heldOrders() const;
    std::vector<Order> getHeldOrders() const { return heldOrders(); }
    bool hasHeldOrders() const { return !heldOrders().empty(); }

    // Resumes the most recently held order
    void resumeLatest();

    // Resumes a specific held order by orderId
    void resume(const std::string& orderId);
    void resumeHeldOrder(size_t index = 0);

    // Loads an existing order for editing (verifying 7-day rule for Paid orders)
    void loadOrderForEdit(const std::string& orderId, bool managerApproved);

    // Sets customer name and contact
    void setCustomer(const std::string& name, const std::string& contact = "-");

    // Sets payment mode and optional merchant
    void setPayment(PaymentMode mode, const std::string& merchant = "-");

    // Access active draft
    const Order& draft() const;
    bool hasActiveDraft() const { return hasDraft; }
    bool isEditingExisting() const { return isEditingExisting_; }

    // Finalizes a new or resumed order
    Order finalize();

    // Finalizes edits to an existing order (updating stock differential and files)
    Order finalizeEdit();

private:
    void requireDraft() const;

    IProductRepository& productRepository;
    ProductService& productService;
    IOrderRepository& orderRepository;
    IOrderDetailRepository& orderDetailRepository;
    IdGenerator& idGenerator;
    const IClock& clock;

    Order draftOrder;
    bool hasDraft = false;
    bool customerIsSet = false;
    bool paymentIsSet = false;
    bool isEditingExisting_ = false;
    std::vector<OrderLine> originalLines_; // Snapshot for stock difference on edit
    std::vector<Order> memoryHeldOrders_;
};

#endif // POS_ORDERSERVICE_HPP