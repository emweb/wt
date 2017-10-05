#include "Address.h"


namespace Wt {
  namespace Payment {

void Address::setName(const WString& name)
{
  name_ = name;
}

void Address::setStreet1(const WString& street1)
{
  street1_ = street1;
}

void Address::setStreet2(const WString& street2)
{
  street2_ = street2;
}

void Address::setCity(const WString& city)
{
  city_ = city;
}

void Address::setState(const WString& state)
{
  state_ = state;
}

void Address::setCountryCode(const std::string& country)
{
  countryCode_ = country;
}

void Address::setZip(const WString& zip)
{
  zip_ = zip;
}

void Address::setPhoneNumber(const std::string& number)
{
  phoneNumber_ = number;
}

  }
}
