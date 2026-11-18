// test_pos_flow.cpp                                  Owner: Person 1
// End-to-end integration test verifying the complete flow from order creation
// through checkout to bill generation and golden file formatting.

#include <cassert>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>

#include "pos/common/IClock.hpp"
#include "pos/common/PosError.hpp"
#include "pos/services/BillingService.hpp"
#include "pos/services/CheckoutService.hpp"
#include "pos/services/IdGenerator.hpp"
#include "pos/services/OrderService.hpp"
#include "pos/services/ProductService.hpp"
#include "pos/storage/FileBillRepository.hpp"
#include "pos/storage/FileOrderDetailRepository.hpp"
#include "pos/storage/FileOrderRepository.hpp"
#include "pos/storage/FileProductRepository.hpp"
#include "pos/storage/ProductCodec.hpp"
#include "test_utils.hpp"

namespace {

class FixedClock : public IClock {
public:
    std::string today() const override { return "2026-11-14"; }
    std::string now() const override { return "10:32:05"; }
    long long millis() const override { return 1000; }
};

std::string readFileContents(const std::string& path) {
    std::ifstream in(path.c_str());
    std::string content;
    std::string line;
    while (std::getline(in, line)) {
        if (!content.empty()) content += "\n";
        content += line;
    }
    return content;
}

void writeFile(const std::string& path, const std::string& content) {
    std::ofstream out(path.c_str());
    out << content;
}

}  // namespace

int main() {
    bool ok = true;

    const std::string productPath = "test_flow_products.txt";
    const std::string orderPath = "test_flow_orders.txt";
    const std::string detailPath = "test_flow_details.txt";
    const std::string billPath = "test_flow_bills.txt";

    // Setup initial Product file with Golden products
    std::string initialProducts =
        "1 100001 TEA Tetley Tea Bags 25 count 350.00 17.50 10.00 40 data/barcodes/100001.bmp#\n"
        "2 100002 SUGAR Sugar Loose weight 180.00 0.00 0.00 25.5 data/barcodes/100002.bmp#\n"
        "3 100003 BISCUIT Sooper Biscuit count 60.00 3.00 5.00 100 data/barcodes/100003.bmp#\n";
    writeFile(productPath, initialProducts);

    // Seed previous records so IdGenerator yields order 123 (00012314) and bill 45 (000000045)
    writeFile(orderPath, "00012214 2026-11-14 10:00:00 Prev 0300 paid 100.00 cash -#\n");
    writeFile(detailPath, "");
    writeFile(billPath, "000000044 Prev 00012214 TEA 350.00 10.00 1 350.00 17.50 10.00 357.50 paid#\n");

    FixedClock clock;
    FileProductRepository productRepo(productPath);
    pos::FileOrderRepository orderRepo(orderPath);
    pos::FileOrderDetailRepository detailRepo(detailPath);
    FileBillRepository billRepo(billPath);

    pos::IdGenerator idGen(orderRepo, billRepo, clock);
    StockPolicy stockPolicy;
    ProductService prodService(productRepo, stockPolicy);
    OrderService orderService(productRepo, prodService, orderRepo, detailRepo, idGen, clock);

    BillingService billingService([&idGen]() { return idGen.nextBillId(); });
    CheckoutService checkoutService(orderService, billingService, billRepo);

    // 1. Test stock enforcement: TEA stock is 40 -> 41 must be rejected
    orderService.start();
    ok &= checkThrowsAs<PosError>("TEA x 41 is rejected", [&]() {
        orderService.addItem("100001", 41.0);
    });

    // 2. Barcode not in Product.txt gives error
    ok &= checkThrowsAs<PosError>("Unknown barcode is rejected", [&]() {
        orderService.addItem("999999", 1.0);
    });

    // 3. 9-character acronym rejected by ProductCodec
    ok &= checkThrowsAs<PosError>("9-character acronym rejected", [&]() {
        std::vector<std::string> badTokens = {
            "1", "100001", "TOOLONGACR", "Test Name", "count", "100.00", "0.00", "0.00", "10", "path.bmp"
        };
        ProductCodec::parse(badTokens);
    });

    // 4. Record golden order: TEA x 2, SUGAR x 1.5, Ali Raza, 0300-1234567, Cash
    orderService.start();
    orderService.addItem("100001", 2.0);
    orderService.addItem("100002", 1.5);
    orderService.setCustomer("Ali Raza", "0300-1234567");
    orderService.setPayment(PaymentMode::Cash, "-");

    Bill bill = checkoutService.checkout();

    // 5. Verify generated bill details
    ok &= checkResult("Bill id is 000000045", "000000045", bill.id);
    ok &= checkResult("Order id is 00012314", "00012314", bill.orderId);
    ok &= checkResult("Customer is Ali Raza", "Ali Raza", bill.customerName);
    ok &= checkResult("Total without tax is 970.00", "970.00", bill.totalWithoutTax.toString());
    ok &= checkResult("Total tax is 35.00", "35.00", bill.totalTax.toString());
    ok &= checkResult("Total discount is 20.00", "20.00", bill.totalDiscount.toString());
    ok &= checkResult("Total payable is 985.00", "985.00", bill.totalPayable.toString());

    // 6. Verify files contain golden lines
    std::string orderContent = readFileContents(orderPath);
    std::string expectedOrderLine = "00012314 2026-11-14 10:32:05 Ali Raza 0300-1234567 paid 985.00 cash -#";
    ok &= checkTrue("Order.txt contains golden line", orderContent.find(expectedOrderLine) != std::string::npos);

    std::string detailContent = readFileContents(detailPath);
    ok &= checkTrue("OrderDetail.txt contains TEA line", detailContent.find("00012314 1 100001 2#") != std::string::npos);
    ok &= checkTrue("OrderDetail.txt contains SUGAR line", detailContent.find("00012314 2 100002 1.5#") != std::string::npos);

    std::string billContent = readFileContents(billPath);
    std::string expectedBillTea = "000000045 Ali Raza 00012314 TEA 350.00 10.00 2 970.00 35.00 20.00 985.00 paid#";
    std::string expectedBillSugar = "000000045 Ali Raza 00012314 SUGAR 180.00 0.00 1.5 970.00 35.00 20.00 985.00 paid#";
    ok &= checkTrue("Bill.txt contains TEA golden line", billContent.find(expectedBillTea) != std::string::npos);
    ok &= checkTrue("Bill.txt contains SUGAR golden line", billContent.find(expectedBillSugar) != std::string::npos);

    // 7. Verify stock reduction in Product.txt
    auto tea = productRepo.findByBarcode("100001");
    ok &= checkTrue("TEA found", tea.has_value());
    if (tea) {
        ok &= checkTrue("TEA stock reduced to 38", tea->availableQty == 38.0);
    }

    auto sugar = productRepo.findByBarcode("100002");
    ok &= checkTrue("SUGAR found", sugar.has_value());
    if (sugar) {
        ok &= checkTrue("SUGAR stock reduced to 24.0", sugar->availableQty == 24.0);
    }

    // Cleanup
    std::remove(productPath.c_str());
    std::remove(orderPath.c_str());
    std::remove(detailPath.c_str());
    std::remove(billPath.c_str());

    if (ok) {
        std::cout << "test_pos_flow: all passed\n";
        return 0;
    }
    std::cout << "test_pos_flow: some tests failed\n";
    return 1;
}
