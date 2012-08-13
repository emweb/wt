#include "Customer"

#include "Wt/WString"

namespace Wt {
  namespace Payment {

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
