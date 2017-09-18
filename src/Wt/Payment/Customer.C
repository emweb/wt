#include "Customer.h"

#include "Wt/WString.h"

namespace Wt {
  namespace Payment {

/*! \defgroup payment Payment module (Wt::Payment)
 *  \brief A module that implements payment functions.
 *
 * This module provides API for interacting with third-party payment
 * brokers.
 *
 * It only provides backend functions (no widgets), which use web
 * services and JavaScript popup windows or redirection to interact
 * with the third party service.
 *
 * The module is organized in a number of utility classes which
 * provide information on the Customer and the Order, and service
 * classes for payment.
 *
 * At the moment, only support for payment using PayPal is implemented
 * in the PayPalService class.
 */

  Customer::Customer()
  {

  }

  void Customer::setFirstName(const WString& firstName)
  {
    firstName_ = firstName;
  }

  void Customer::setLastName(const WString& lastName)
  {
    lastName_ = lastName;
  }

  void Customer::setEmail(const std::string& email)
  {
    email_ = email;
  }

  void Customer::setShippingAddress(const Address& address)
  {
    shippingAddress_ = address;
  }

  void Customer::setLocale(const std::string& locale)
  {
    locale_ = locale;
  }

  void Customer::setPayerId(const std::string& payerId)
  {
    payerId_ = payerId;
  }

  }
}
