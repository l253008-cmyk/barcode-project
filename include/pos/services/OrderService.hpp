#ifndef POS_ORDERSERVICE_HPP
#define POS_ORDERSERVICE_HPP

#include <string>
#include "pos/common/IClock.hpp"
#include "pos/domain/Order.hpp"
#include "pos/services/IdGenerator.hpp"
#include "pos/services/ProductService.hpp"
#include "pos/storage/IOrderDetailRepository.hpp"
#include "pos/storage/IOrderRepository.hpp"
#include "pos/storage/IProductRepository.hpp"

// builds one order step by step (the draft) and saves it at the end
class OrderService {
public:
    OrderService(IProductRepository& products,
        ProductService& productService,
        IOrderRepository& orders,
        IOrderDetailRepository& orderDetails,
        IdGenerator& idGenerator,
        const IClock& clock);

    // starts a new empty order, an old unfinished one is thrown away
    void start();

    // adds a product by barcode, the price comes from the product
    // scanning the same product again adds to its quantity
    // error if the barcode is unknown or the stock is not enough
    void addItem(const std::string& barcode, double quantity);

    // takes a product out of the order
    void removeItem(const std::string& barcode);

    // changes the quantity of an existing line item in the order
    // error if quantity <= 0, item not in order, or stock not enough
    void updateItemQuantity(const std::string& barcode, double newQuantity);

    // puts the active order on hold and starts a new empty order
    void holdCurrentOrder();

    // true if there is at least one order on hold
    bool hasHeldOrders() const;

    // returns all held orders
    std::vector<Order> getHeldOrders() const;

    // restores a held order (by index, defaults to 0)
    void resumeHeldOrder(size_t index = 0);

    // name is needed, contact is optional (empty becomes "-")
    void setCustomer(const std::string& name, const std::string& contact);

    // merchant is optional (empty becomes "-")
    void setPayment(PaymentMode mode, const std::string& merchant);

    // the order as it is right now
    const Order& draft() const;

    // true if there is an active order being edited
    bool hasActiveDraft() const { return hasDraft; }

    // saves the order and its lines, reduces the stock, gives back the saved order
    // nothing is saved if something is missing or the stock is not enough any more
    Order finalize();

private:
    void requireDraft() const;

    IProductRepository& productRepository;
    ProductService& productService;
    IOrderRepository& orderRepository;
    IOrderDetailRepository& orderDetailRepository;
    IdGenerator& idGenerator;
    const IClock& clock;

    Order draftOrder;
    bool hasDraft;
    bool customerIsSet;
    bool paymentIsSet;
    std::vector<Order> heldOrders;
};

#endif