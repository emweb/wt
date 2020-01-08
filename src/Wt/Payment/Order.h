// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2012 Emweb bv, Herent, Belgium.
 */
#ifndef WT_PAYMENT_ORDER_H
#define WT_PAYMENT_ORDER_H

#include <Wt/Payment/OrderItem.h>
#include "Wt/Payment/Money.h"

namespace Wt {
  namespace Payment {
/*! \class Order Wt/Payment/Order.h Wt/Payment/Order.h
 *  \brief Contains information of a sales order.
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
 * \ingroup payment
 */
class WT_API Order
{
public:
  /*! \brief Sets sales taxes.
   *
   * This is the total amount of taxes for the order.
   *
   * The default value is 0 (your government may not like that !).
   */
  void setTax(const Money& tax);

  /*! \brief Returns sales taxes.
   *
   * \sa setTax()
   */
  Money tax() const { return tax_; }

  /*! \brief Sets shipping cost.
   *
   * This is the total shipping cost for the order, excluding
   * discounts and insurance.
   *
   * The default value is 0.
   *
   * \sa setShippingInsurance(), setShippingDiscount()
   */
  void setShipping(const Money& shipping);

  /*! \brief Returns the shipping cost.
   *
   * \sa setShipping()
   */
  Money shipping() const { return shipping_; }

  /*! \brief Sets handling cost.
   *
   * This is the total cost for handling.
   *
   * The default value is 0.
   */
  void setHandling(const Money& handling);

  /*! \brief Returns handeling.
   *
   * \sa setHandling()
   */
  Money handling() const { return handling_; }

  /*! \brief Sets the shipping discount.
   *
   * The shipping order discount (which should be a negative number).
   *
   * The default value is 0.
   */
  void setShippingDiscount(const Money& discount);

  /*! \brief Returns the shipping discount.
   *
   * \sa setShippingDiscount()
   */
  Money shippingDiscount() const { return shippingDiscount_; }

  /*! \brief Sets shipping insurance.
   *
   * The total order shipping insurance cost.
   *
   * The default value is 0.
   */
  void setShippingInsurance(const Money& insurance);

  /*! \brief Returns shipping insurance.
   *
   * \sa setShippingInsurance()
   */
  Money shippingInsurance() const { return shippingInsurance_; }

  /*! \brief Sets a payment transaction ID.
   *
   * This transaction identification number provides tracability of the
   * payment, and is usually set by the payment broker during the payment
   * process.
   *
   * \sa setPaymentBroker()
   */
  void setPaymentTransactionId(const std::string& transactionId);

  /*! \brief Returns the payment transaction ID.
   *
   * \sa setPaymentTransactionId()
   */
  std::string paymentTransactionId() const { return paymentTransactionId_; }

  /*! \brief Sets the payment broker.
   *
   * This identifies the payment broker that was used to provide
   * payment for this order. This is usually set together with a
   * payment transaction ID by the payment broker during the payment
   * process.
   *
   * \sa setPaymentTransactionId()
   */
  void setPaymentBroker(const std::string& broker);

  /*! \brief Returns the payment broker.
   *
   * \sa setPaymentBroker()
   */
  std::string paymentBroker() const { return paymentBroker_; }

  /*! \brief Sets the order description.
   *
   * Sets a description for the total order.
   */
  void setDescription(const std::string description);

  /*! \brief Returns the order description.
   *
   * \sa setDescription()
   */
  std::string description() const {return description_;}

  /*! \brief Returns all items in the order.
   *
   * \sa OrderItem
   */
  const std::vector<OrderItem>& items() const { return items_; }

  /*! \brief Returns all items in the order.
   *
   * \sa OrderItem
   */  
  std::vector<OrderItem>& items() { return items_; }

  /*! \brief Computes the total net cost of all items.
   *
   * This computes the total item cost based on the list of items() in
   * this order. This does not include tax, shipping,
   * shippingDiscount, shippingInsurance and handling fees.
   *
   * \sa items(), setTotalItemCost()
   */
  Money computeTotalItemCost() const;

  /*! \brief Sets the total net cost of all items.
   *
   * This sets the total item cost. This does not include tax, shipping,
   * shippingDiscount, shippingInsurance and handling fees.
   *
   * If you have specified the individual items, then you can use
   * computeTotalItemCost() to set the computed value.
   */
  void setTotalItemCost(const Money& totalItemCost);

  /*! \brief Returns the total item cost.
   *
   * \sa setTotalItemCost()
   */
  Money totalItemCost() const { return totalItemCost_; }

  /*! \brief Computes the total cost of the order.
   *
   * Computes the total of totalItemCost(), tax(), shipping(),
   * handling(), shippinDiscount() and shippingInsurance().
   *
   * \sa setTotalOrderCost()
   */
  Money computeTotalOrderCost() const;

  /*! \brief Sets the total order cost.
   *
   * This sets the total order cost. If you have specified the total
   * item cost, and suitable values for tax, shipping and handling,
   * then you can use computeTotalOrderCost() to set the computed
   * value.
   */
  void setTotalOrderCost(const Money& totalOrderCost);

  /*! \brief Returns the total order cost.
   */
  Money totalOrderCost() const { return totalCost_; }

private:
  std::vector<OrderItem> items_;
  Money tax_, shipping_, shippingDiscount_, shippingInsurance_,
    handling_, totalCost_, totalItemCost_;
  std::string paymentTransactionId_, paymentBroker_, description_;
};

  }
}

#endif // WT_PAYMENT_ORDER_H_
