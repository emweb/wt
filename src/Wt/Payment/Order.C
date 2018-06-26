#include "Order.h"

namespace Wt {
  namespace Payment {

  void Order::setTax(const Money& tax)
  {
    tax_ = tax;
  }

  void Order::setShipping(const Money& shipping)
  {
    shipping_ = shipping;
  }

  void Order::setHandling(const Money& handling)
  {
    handling_ = handling;
  }

  void Order::setShippingDiscount(const Money& discount)
  {
    shippingDiscount_ = discount;
  }

  void Order::setShippingInsurance(const Money& insurance)
  {
    shippingInsurance_ = insurance;
  }

  void Order::setPaymentTransactionId(const std::string& transactionId)
  {
    paymentTransactionId_ = transactionId;
  }

  void Order::setPaymentBroker(const std::string& broker)
  {
    paymentBroker_ = broker;
  }

  void Order::setDescription(const std::string description)
  {
    description_ = description;
  }

  void Order::setTotalItemCost(const Money& totalItemCost)
  {
    totalItemCost_ = totalItemCost;
  }

  void Order::setTotalOrderCost(const Money& totalCost)
  {
    totalCost_ = totalCost;
  }

  Money Order::computeTotalItemCost() const
  {
    if (items_.size() == 0)
      return Money();

    OrderItem item = items_[0];
    Money ans = item.computeTotal();
    for (unsigned i = 1; i < items().size(); ++i){
      item = items_[i];
      ans+= item.computeTotal();
    }

    return ans;
  }

  Money Order::computeTotalOrderCost() const
  {
    return computeTotalItemCost() + tax_ + shipping_ + shippingDiscount_ +
            shippingInsurance_ + handling_;
  }

  }
}
