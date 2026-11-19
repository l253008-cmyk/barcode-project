﻿#include "pos/services/BillingService.hpp"
#include "pos/common/PosError.hpp"
#include "test_utils.hpp"
#include <iostream>
#include <cstdio>

int main() {
    bool ok = true;
    int nextIdCounter = 1;
    auto idGen = [&nextIdCounter]() {
        char buf[32];
        snprintf(buf, sizeof(buf), "%09d", nextIdCounter++);
        return std::string(buf);
        };

    BillingService service(idGen);

    // Test 1: Valid order calculation matching Section 10 golden data
    {
        Order order;
        order.id = "00012314";
        order.customerName = "Ali Raza";
        order.status = OrderStatus::Paid;

        OrderLine line1;
        line1.acronym = "TEA";
        line1.unitPrice = Money::parse("350.00");
        line1.tax = Money::parse("17.50");
        line1.discount = Money::parse("10.00");
        line1.quantity = 2.0;

        OrderLine line2;
        line2.acronym = "SUGAR";
        line2.unitPrice = Money::parse("180.00");
        line2.tax = Money::parse("0.00");
        line2.discount = Money::parse("0.00");
        line2.quantity = 1.5;

        order.lines.push_back(line1);
        order.lines.push_back(line2);

        Bill bill = service.createBill(order);

        ok &= checkResult("Bill id", "000000001", bill.id);
        ok &= checkResult("Order id", "00012314", bill.orderId);
        ok &= checkResult("Customer", "Ali Raza", bill.customerName);
        ok &= checkResult("Status", "paid", bill.status);
        ok &= checkTrue("Line count", bill.lines.size() == 2);

        ok &= checkResult("Total without tax", "970.00", bill.totalWithoutTax.toString());
        ok &= checkResult("Total tax", "35.00", bill.totalTax.toString());
        ok &= checkResult("Total discount", "20.00", bill.totalDiscount.toString());
        ok &= checkResult("Total payable", "985.00", bill.totalPayable.toString());
    }

    // Test 2: Empty order throws PosError
    {
        Order emptyOrder;
        ok &= checkThrowsAs<PosError>("Empty order throws", [&]() {
            service.createBill(emptyOrder);
            });
    }

    // Test 3: Zero or negative quantity throws PosError
    {
        Order order;
        order.id = "00012314";
        OrderLine line;
        line.acronym = "TEA";
        line.unitPrice = Money::parse("100.00");
        line.quantity = 0.0;
        order.lines.push_back(line);

        ok &= checkThrowsAs<PosError>("Zero quantity throws", [&]() {
            service.createBill(order);
            });

        order.lines[0].quantity = -1.0;
        ok &= checkThrowsAs<PosError>("Negative quantity throws", [&]() {
            service.createBill(order);
            });
    }

    if (ok) {
        std::cout << "test_billing_service: all passed\n";
        return 0;
    }
    std::cout << "test_billing_service: some tests failed\n";
    return 1;
}