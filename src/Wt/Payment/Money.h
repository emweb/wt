// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2012 Emweb bv, Herent, Belgium.
 */
#ifndef WT_PAYMENT_MONEY_H
#define WT_PAYMENT_MONEY_H

#include <string>
#include <Wt/WStringStream.h>

namespace Wt {
  namespace Payment {

/*! \class Money Wt/Payment/Money.h Wt/Payment/Money.h
 *  \brief A value class which describes a monetary value.
 *
 * The money is stored in cents as a 64-bit number, which allows for
 * accurate representation for values up to 2^61 - 1, but has as
 * consequence that division will round to the nearest cent.
 *
 * Math operations on monetary values of different currencies are not
 * allowed and will result in exception.
 *
 * \ingroup payment
 */
class WT_API Money
{
public:
  /*! \brief Default constructor.
   *
   * Creates money represent 0 value. This value can be added or
   * substracted safely from any other monetary value (unlike Money(0,
   * 0, "EUR") which would only be safely used in operations involving
   * other money expressed in the Euro currency.
   */
  Money();

  /*! \brief Creates a monetary value.
   *
   * The \p value is the integer value, \p cents is the fractional
   * value (up to 2 digits) expressed in cents (0 - 100)
   * and the \p currency is a string which indicates the currency.
   */
  Money(long long value, int cents, const std::string& currency);

  /*! \brief Returns the int part of money.
   *
   * Returns the int part of money (money with no cents).
   */
  long long value() const {return valueInCents_ / 100;}

  /*! \brief Returns the cents.
   *
   * Returns the cents - the last 2 digits of value in cents.
   */
  int cents() const {return static_cast<int>(valueInCents_ % 100); }

  /*! \brief Sets the currency.
   */
  void setCurrency(std::string currency) {currency_ = currency;}

  /*! \brief Returns the currency.
   *
   * \sa setCurrency()
   */
  std::string currency() const {return currency_;}

  /*! \brief Returns a text representation.
   *
   * The format is "value.cents".
   */
  const std::string toString() const;

  /*! \brief Assignment operator
   */
  Money& operator= (const Money& money);

  /*! \brief Addition operator
   *
   * Adding money of different currencies is not allowed.
   */
  Money& operator+= (const Money& money);

  /*! \brief Substraction operator
   *
   * Subtracting money of different currencies is not allowed.
   */
  Money& operator-= (const Money& money);

  /*! \brief Multiplication operator
   */
  Money& operator*= (double value);

  /*! \brief Division operator
   */
  Money& operator/= (double value);

  /*! \brief Multiplication operator
   */
  Money& operator*= (unsigned value);

  /*! \brief Division operator
   */
  Money& operator/= (unsigned value);

private:
  long long valueInCents_;
  std::string currency_;

  Money(long long valueInCents, const std::string &currency);
  long long valueInCents() const {return valueInCents_;}
  void checkCurrency(Money& ans, const Money& v1, const Money& v2);
};

/*! \brief Adds monetary values.
 *
 * Adding money of different currencies is not allowed.
 *
 * \ingroup payment
 */
extern WT_API Money operator+ (const Money& v1, const Money& v2);

/*! \brief Substact monetary values.
 *
 * Subtraction of money of different currencies is not allowed.
 *
 * \ingroup payment
 */
extern WT_API Money operator- (const Money& v1, const Money& v2);

/*! \brief Multiplies money.
 *
 * \ingroup payment
 */
extern WT_API Money operator* (const Money& v1, double v2);

/*! \brief Multiplies money.
 *
 * You would wish you could just do that, wouldn't you ?
 *
 * \ingroup payment
 */
extern WT_API Money operator* (double v1, const Money& v2);

/*! \brief Divides v1 and v2
 *
 * \ingroup payment
 */
extern WT_API Money operator/ (const Money& v1, double v2);

  }
}

#endif // WT_PAYMENT_MONEY_H_
