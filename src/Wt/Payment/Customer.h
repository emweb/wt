// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2012 Emweb bv, Herent, Belgium.
 */
#ifndef WT_PAYMENT_CUSTOMER_H
#define WT_PAYMENT_CUSTOMER_H

#include <Wt/Payment/Address.h>
#include <string>

namespace Wt {
  class WString;
  namespace Payment {

/*! \class Customer Wt/Payment/Customer.h Wt/Payment/Customer.h
 *  \brief Contains customer information.
 *
 * \if cpp
 * \code
 *   Wt::Payment::Customer customer;
 *
 *   customer.setEmail("joe.birkenberg@emweb.be");
 *   customer.setFirstName("Joe");
 *   customer.setLastName("Birkenberg");
 *
 *   Wt::Payment::Address address;
 *   address.setCity("Leuven");
 *   address.setCountryCode("BE");
 *   address.setPhoneNumber("123456789");
 *   address.setStreet1("Brusselsestraat 14");
 *
 *   customer.setShippingAddress(address);
 * \endcode
 * \endif
 *
 * \ingroup payment
 */
class WT_API Customer
{
public:
  /*! \brief Default constructor.
   *
   * All information is blank.
   */
  Customer();

  /*! \brief Sets the first name.
   */
  void setFirstName(const WString& firstName);

  /*! \brief Returns the first name.
   *
   * \sa setFirstName()
   */
  WString firstName() const { return firstName_; }

  /*! \brief Sets the last name.
   */
  void setLastName(const WString& lastName);

  /*! \brief Returns the last name.
   *
   * \sa setLastName()
   */
  WString lastName() const { return lastName_; }

  /*! \brief Sets the email address.
   */
  void setEmail(const std::string& email);

  /*! \brief Returns the email address.
   *
   * \sa setEmail()
   */
  std::string email() const { return email_; }

  /*! \brief Sets the shipping address.
   */
  void setShippingAddress(const Address& address);

  /*! \brief Returns shipping address.
   *
   * \sa setShippingAddress()
   */
  const Address& shippingAddress() const { return shippingAddress_; }

  /*! \brief Sets the customer locale.
   *
   * The customer locale must be specified according to the payment broker
   * (usually to help the user being served in his native language), which
   * is usually a language code like http://en.wikipedia.org/wiki/BCP_47
   */
  void setLocale(const std::string& locale);

  /*! \brief Returns locale
   *
   * \sa setLocale()
   */
  std::string locale() const { return locale_; }

  /*! \brief Sets the payerId field
   *
   * This is the identification of the user with a payment broker which also
   * keeps login information (and other information like shipping addresses)
   * on the user.
   *
   * Not all payment brokers support (or need this).
   */
  void setPayerId(const std::string& payerId);

  /*! \brief Returns payerId
   *
   * \sa setPayerId()
   */
  std::string payerId() const { return payerId_; }

private:
  WString firstName_, lastName_;
  std::string email_;
  Address shippingAddress_;
  std::string locale_;
  std::string payerId_;
};

  }
}

#endif // WT_PAYMENT_CUSTOMER_H_
