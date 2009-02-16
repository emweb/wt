#include "DateValidator.h"

#include <Wt/WString>
#include <boost/date_time/gregorian/gregorian.hpp>

using namespace boost::gregorian;

/*
 * Disclaimer: I am clueless how to use boost::gregorian in a sensible way.
 *
 * That, together with the fact that I wanted to test WRegExpValiator
 * is the reason why I use a regular expression to get the
 * day/month/year fields, and boost::gregorian to check that the date
 * is a valid date.
 */

DateValidator::DateValidator(const date& bottom, const date& top)
  : WRegExpValidator("(\\d{1,2})/(\\d{1,2})/(\\d{4})"),
    bottom_(bottom),
    top_(top)
{ 
  setNoMatchText("Must be a date in format 'dd/MM/yyyy'");
}

WValidator::State DateValidator::validate(WString& input, int& pos) const
{
  WValidator::State state = WRegExpValidator::validate(input, pos);

  std::string text = input.toUTF8();

  if ((state == Valid) && !text.empty()) {
    boost::smatch what;
    boost::regex_match(text, what, regExp());

    try {
      date d
	= from_string(what[3] + "/" + what[2] + "/" + what[1]);

      if ((d >= bottom_) && (d <= top_))
	return Valid;
      else
	return Invalid;

    } catch (std::exception& e) {
      return Invalid;
    }
  } else
    return state;
}
