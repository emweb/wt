#include "OrderItem.h"

namespace Wt {
  namespace Payment {

  void OrderItem::setName(const WString& name)
  {
    name_ = name;
  }

  void OrderItem::setNumber(const std::string& number)
  {
    number_ =number;
  }

  void OrderItem::setDescription(const WString& description)
  {
    description_ = description;
  }

  void OrderItem::setQuantity(double quantity)
  {
    quantity_= quantity;
  }

  void OrderItem::setUnitCost(const Money& unitCost)
  {
    unitCost_ = unitCost;
  }

  Money OrderItem::computeTotal() const
  {
    return unitCost_ * quantity_;
  }

  }
}
