#include <cassert>
#include <iostream>
#include "pos/common/Money.hpp"
#include "pos/common/PosError.hpp"
#include "pos/services/ProductService.hpp"
#include "pos/services/StockPolicy.hpp"
#include "pos/storage/IProductRepository.hpp"
using namespace std;

// fake repository, keeps the products in a list so no file is needed
class FakeProductRepository : public IProductRepository {
public:
    vector<Product> products;

    vector<Product> all() const override {
        return products;
    }

    optional<Product> findByBarcode(const string& barcode) const override {
        for (const Product& product : products) {
            if (product.barcode == barcode) {
                return product;
            }
        }
        return nullopt;
    }

    optional<Product> findById(int id) const override {
        for (const Product& product : products) {
            if (product.id == id) {
                return product;
            }
        }
        return nullopt;
    }

    void updateQuantity(int id, double newQuantity) override {
        for (Product& product : products) {
            if (product.id == id) {
                product.availableQty = newQuantity;
            }
        }
    }
};

// the three products from the proposal
static Product makeTea() {
    Product tea = { 1, "100001", "TEA", "Tetley Tea Bags 25", MeasurementUnit::Count,
                   Money::parse("350.00"), Money::parse("17.50"), Money::parse("10.00"),
                   40, "data/barcodes/100001.bmp" };
    return tea;
}

static Product makeSugar() {
    Product sugar = { 2, "100002", "SUGAR", "Sugar Loose", MeasurementUnit::Weight,
                     Money::parse("180.00"), Money::parse("0.00"), Money::parse("0.00"),
                     25.5, "data/barcodes/100002.bmp" };
    return sugar;
}

static Product makeBiscuit() {
    Product biscuit = { 3, "100003", "BISCUIT", "Sooper Biscuit", MeasurementUnit::Count,
                       Money::parse("60.00"), Money::parse("3.00"), Money::parse("5.00"),
                       100, "data/barcodes/100003.bmp" };
    return biscuit;
}

// these helpers give true when the call throws a PosError

static bool describeThrows(const ProductService& service, const string& barcode) {
    try {
        service.describeByBarcode(barcode);
    }
    catch (const PosError&) {
        return true;
    }
    return false;
}

static bool canSellThrows(const ProductService& service, const string& barcode, double quantity) {
    try {
        service.canSell(barcode, quantity);
    }
    catch (const PosError&) {
        return true;
    }
    return false;
}

static bool reduceStockThrows(ProductService& service, const string& barcode, double quantity) {
    try {
        service.reduceStock(barcode, quantity);
    }
    catch (const PosError&) {
        return true;
    }
    return false;
}

int main() {
    Product tea = makeTea();
    Product sugar = makeSugar();

    // ---------- StockPolicy ----------
    StockPolicy policy;
    assert(policy.canFulfil(tea, 1));
    assert(policy.canFulfil(tea, 40));             // last unit can be sold
    assert(!policy.canFulfil(tea, 41));            // more than the stock
    assert(!policy.canFulfil(tea, 0));             // nothing
    assert(!policy.canFulfil(tea, -1));            // negative
    assert(!policy.canFulfil(tea, 1.5));           // count product, not a whole number
    assert(policy.canFulfil(sugar, 1.5));          // weight product, decimals are fine
    assert(policy.canFulfil(sugar, 25.5));
    assert(!policy.canFulfil(sugar, 25.6));
    assert(!policy.canFulfil(sugar, 0));

    // ---------- ProductService ----------
    FakeProductRepository repository;
    repository.products.insert(repository.products.end(), 1, makeTea());
    repository.products.insert(repository.products.end(), 1, makeSugar());
    repository.products.insert(repository.products.end(), 1, makeBiscuit());
    ProductService service(repository, policy);

    // describeByBarcode
    string text = service.describeByBarcode("100001");
    assert(text.find("Tetley Tea Bags 25") != string::npos);
    assert(text.find("TEA") != string::npos);
    assert(text.find(tea.unitPrice.toString()) != string::npos);
    assert(text.find("Available: 40") != string::npos);
    assert(service.describeByBarcode("100002").find("Available: 25.5") != string::npos);
    assert(describeThrows(service, "999999"));     // not in the list
    assert(describeThrows(service, ""));

    // canSell
    assert(service.canSell("100001", 40));
    assert(!service.canSell("100001", 41));
    assert(!service.canSell("100001", 0));
    assert(service.canSell("100002", 1.5));
    assert(canSellThrows(service, "999999", 1));

    // reduceStock
    service.reduceStock("100001", 2);
    assert(repository.findById(1).value().availableQty == 38);
    service.reduceStock("100002", 1.5);
    assert(repository.findById(2).value().availableQty == 24);

    assert(reduceStockThrows(service, "100001", 41));          // too much
    assert(repository.findById(1).value().availableQty == 38); // stock did not change
    assert(reduceStockThrows(service, "999999", 1));           // unknown barcode

    service.reduceStock("100001", 38);                         // sell everything
    assert(repository.findById(1).value().availableQty == 0);
    assert(!service.canSell("100001", 1));                     // sold out now

    cout << "test_product_service: all passed" << endl;
    return 0;
}