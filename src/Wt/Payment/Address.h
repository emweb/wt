// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2012 Emweb bv, Herent, Belgium.
 */
#ifndef WT_PAYMENT_ADDRESS_H
#define WT_PAYMENT_ADDRESS_H

#include <string>

#include <Wt/WString.h>

namespace Wt {
  namespace Payment {
/*! \class Address Wt/Payment/Address.h Wt/Payment/Address.h
 *  \brief Contains address information.
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
 * \sa Customer::setShippingAddress()
 *
 * \ingroup payment
 */
class WT_API Address
{
public:
  /*! \brief Sets an address name.
   *
   * Sets a name for this address like "home" or "work".
   * This can allow a user to maintain several addresses.
   */
  void setName(const WString& name);

  /*! \brief Returns the address name.
   *
   * \sa setName()
   */
  const WString& name() const { return name_; }

  /*! \brief Sets first street line.
   *
   * \sa setStreet2()
   */
  void setStreet1(const WString& street1);

  /*! \brief Returns the first street line.
   *
   * \sa setStreet1()
   */
  const WString& street1() const { return street1_; }

  /*! \brief Sets the second street line.
   *
   * \sa setStreet1()
   */
  void setStreet2(const WString& street2);

  /*! \brief Returns the second street line.
   *
   * \sa setStreet2()
   */
  const WString& street2() const { return street2_; }

  /*! \brief Sets the city.
   */
  void setCity(const WString& city);

  /*! \brief Returns the city.
   *
   * \sa setCity()
   */
  const WString& city() const { return city_; }

  /*! \brief Sets the state.
   */
  void setState(const WString& state);

  /*! \brief Returns the state.
   *
   * \sa setState()
   */
  const WString& state() const { return state_; }

  /*! \brief Sets the country code.
   */
  void setCountryCode(const std::string& country);

  /*! \brief Returns the country code.
   *
   * \sa setCountryCode()
   */
  const std::string& countryCode() const { return countryCode_; }

  /*! \brief Sets the zip code.
   */
  void setZip(const WString& zip);

  /*! \brief Returns the zip code.
   *
   * \sa setZip()
   */
  const WString& zip() const { return zip_; }

  /*! \brief Sets the phone number.
   */
  void setPhoneNumber(const std::string& number);

  /*! \brief Returns the phone number.
   *
   * \sa setPhoneNumber()
   */
  const std::string& phoneNumber() const { return phoneNumber_; }

private:
  std::string countryCode_, phoneNumber_;
  WString name_, street1_, street2_, city_, state_, zip_;
};

  }
}

#endif // WT_PAYMENT_ADDRESS_H_
