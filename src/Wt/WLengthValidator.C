/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/lexical_cast.hpp>

#include "Wt/WLengthValidator"

namespace Wt {

WLengthValidator::WLengthValidator(WObject *parent)
  : WValidator(parent),
    minLength_(0),
    maxLength_(std::numeric_limits<int>::max())
{ }

WLengthValidator::WLengthValidator(int minLength, int maxLength,
				   WObject *parent)
  : WValidator(parent),
    minLength_(minLength),
    maxLength_(maxLength)
{ }

void WLengthValidator::setMinimumLength(int minLength)
{
  if (minLength_ != minLength) {
    minLength_ = minLength;
    repaint();
  }
}

void WLengthValidator::setMaximumLength(int maxLength)
{
  if (maxLength_ != maxLength) {
    maxLength_ = maxLength;
    repaint();
  }
}

void WLengthValidator::setInvalidTooShortText(const WString& text)
{
  tooShortText_ = text;
  repaint();
}

WString WLengthValidator::invalidTooShortText() const
{
  if (!tooShortText_.empty()) {
    WString s = tooShortText_;
    s.arg(minLength_).arg(maxLength_);
    return s;
  } else
    if (minLength_ == 0)
      return WString();
    else
      if (maxLength_ == std::numeric_limits<int>::max())
	return WString::fromUTF8("The input must be at least "
				 + boost::lexical_cast<std::string>(minLength_)
				 + " characters");
      else
	return WString::fromUTF8("The input must have a length between "
				 + boost::lexical_cast<std::string>(minLength_)
				 + " and "
				 + boost::lexical_cast<std::string>(maxLength_)
				 + " characters");
}

void WLengthValidator::setInvalidTooLongText(const WString& text)
{
  tooLongText_ = text;
  repaint();
}

WString WLengthValidator::invalidTooLongText() const
{
  if (!tooLongText_.empty()) {
    WString s = tooLongText_;
    s.arg(minLength_).arg(maxLength_);
    return s;
  } else
    if (maxLength_ == std::numeric_limits<int>::max())
      return WString();
    else
      if (minLength_ == 0)
	return WString::fromUTF8("The input must be no more than "
				 + boost::lexical_cast<std::string>(maxLength_)
				 + " characters");
      else
	return WString::fromUTF8("The input must have a length between "
				 + boost::lexical_cast<std::string>(minLength_)
				 + " and "
				 + boost::lexical_cast<std::string>(maxLength_)
				 + " characters");
}

WValidator::State WLengthValidator::validate(WString& input, int& pos) const
{
  std::wstring text = input.value();

  if (isMandatory()) {
    if (text.empty())
      return InvalidEmpty;
  } else {
    if (text.empty())
      return Valid;
  }

  if ((int)text.length() >= minLength_
      && (int)text.length() <= maxLength_)
    return Valid;
  else
    return Invalid;
  
  return Valid;
}

std::string WLengthValidator::javaScriptValidate(const std::string& jsRef) const
{
  std::string js = "function(e,te,ts,tb){if(e.value.length==0)";

  if (isMandatory())
    js += "return {valid:false,message:te};";
  else
    js += "return {valid:true};";

  if (minLength_ != 0)
    js += "if(e.value.length<" + boost::lexical_cast<std::string>(minLength_)
      + ") return {valid:false,message:ts};";
  if (maxLength_ != std::numeric_limits<int>::max())
    js += "if(e.value.length>" + boost::lexical_cast<std::string>(maxLength_)
      + ") return {valid:false,message:tb};";

  js += "return {valid:true};}(" + jsRef + ','
    + invalidBlankText().jsStringLiteral() + ','
    + invalidTooShortText().jsStringLiteral() + ','
    + invalidTooLongText().jsStringLiteral() + ')';

  return js;
}

void WLengthValidator::createExtConfig(std::ostream& config) const
{
  if (minLength_ != 0) {
    config << ",minLength:" << minLength_;
    if (!tooShortText_.empty())
      config << ",minLengthText:" << tooShortText_.jsStringLiteral();
  }

  if (maxLength_ != std::numeric_limits<int>::max()) {
    config << ",maxLength:" << maxLength_;
    if (!tooLongText_.empty())
      config << ",maxLengthText:" << tooLongText_.jsStringLiteral();
  }

  WValidator::createExtConfig(config);
}

}
