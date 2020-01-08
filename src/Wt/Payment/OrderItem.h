// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2012 Emweb bv, Herent, Belgium.
 */
#ifndef WT_PAYMENT_ORDER_ITEM_H
#define WT_PAYMENT_ORDER_ITEM_H

#include <string>
#include <Wt/WString.h>

#include "Wt/Payment/Money.h"

namespace Wt {
  namespace Payment {

/*! \class OrderItem Wt/Payment/OrderItem.h Wt/Payment/OrderItem.h
 *  \brief Describes an item in an order
 *
 * \if cpp
 * Usage example:
 * \code
 * Wt::Payment::OrderItem item1, item2;
 *
 * item1.setName("Waffle Maker");
 * item1.setNumber("00001");
 * item1.setDescription("Emweb FlipSide Belgian Waffle Maker");
 * item1.setQuantity(1);
 * item1.setUnitCost(Wt::Payment::Money(49, 99, "USD"));
 *
 * Wt::Payment::Order order;
 *
 * order.items().push_back(item1);
 *
 * order.setShipping(Wt::Payment::Money(7, 1, "USD"));
 * order.setShippingDiscount(Wt::Payment::Money(-7, 0, "USD"));
 * order.setShippingInsurance(Wt::Payment::Money(2, 23, "USD"));
 * order.setTax(Wt::Payment::Money(500, 99, "USD"));
 * \endcode
 * \endif
 *
 * \sa Order
 *
 * \ingroup payment
 */
class WT_API OrderItem
{
public:
  /*! \brief Sets the item name.
   */
  void setName(const WString& name);

  /*! \brief Returns item name.
   *
   * \sa setName()
   */
  const WString& name() const { return name_; }

  /*! \brief Sets the item number.
   */
  void setNumber(const std::string& number);

  /*! \brief Returns item number.
   *
   * \sa setNumber()
   */
  std::string number() const { return number_; }

  /*! \brief Sets the item description.
   */
  void setDescription(const WString& description);

  /*! \brief Returns the item description.
   *
   * \sa setDescription()
   */
  const WString& description() const { return description_; }

  /*! \brief Sets the item quantity
   *
   * This is either an integer quantity (number of items) or a
   * fractional quantity (e.g. 1.5 times 1 kilogram).
   *
   * The total price for this item in the order will be the quantity()
   * times the unitCost().
   */
  void setQuantity(double quantity);

  /*! \brief Returns the item quantity
   *
   * \sa setQuantity()
   */
  double quantity() const { return quantity_; }

  /*! \brief Changes the item unit cost field
   *
   * This is the unit cost.
   */
  void setUnitCost(const Money& unitCost);

  /*! \brief Returns item unit cost
   *
   * \sa setUnitCost()
   */
  Money unitCost() const { return unitCost_; }

  /*! \brief Returns the total cost for this order item.
   *
   * This returns quantity() * unitCost()
   */
  Money computeTotal() const;

private:
  WString name_, description_;
  std::string number_;
  double quantity_;
  Money unitCost_;
};

  }
}

#endif // WT_PAYMENT_ORDERITEM_H_
