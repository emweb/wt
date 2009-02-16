// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef DATE_VALIDATOR_H_
#define DATE_VALIDATOR_H_

#include <boost/date_time/gregorian/gregorian_types.hpp>

#include <Wt/WRegExpValidator>

using namespace Wt;

/**
 * @addtogroup formexample
 */
/*@{*/

/*! \brief A validator that accepts dates.
 *
 * This example validator only accepts input in the dd/mm/yyyy format,
 * and checks that the date is in the right range.
 *
 * It would be a natural thing to extend this class to provide
 * access to the parsed date as a boost::gregorian::date object
 * for example.
 *
 * This class is part of the %Wt form example.
 */
class DateValidator : public WRegExpValidator
{
public:
  /*! \brief Construct a date validator.
   *
   * The validator will accept only dates in the indicated range.
   */
  DateValidator(const boost::gregorian::date& bottom,
		const boost::gregorian::date& top);

  /*
   * Reimplement the validate method to check the validity of
   * input as an existing date.
   */
  virtual State validate(WString& input, int& pos) const;

private:
  boost::gregorian::date bottom_, top_;
};

/*@}*/

#endif // DATE_VALIDATOR_H_
