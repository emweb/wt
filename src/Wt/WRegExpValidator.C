/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/regex.hpp>

#include "Wt/WRegExpValidator"
#include "Wt/WString"

#include "Utils.h"

namespace Wt {

struct WRegExpValidatorImpl
{
  boost::regex rx;
  WString      noMatchText_;

  WRegExpValidatorImpl(const boost::regex& arx)
    : rx(arx) { }
};

WRegExpValidator::WRegExpValidator(WObject *parent)
  : WValidator(parent),
    impl_(new WRegExpValidatorImpl(boost::regex("")))
{ }

WRegExpValidator::WRegExpValidator(const boost::regex& rx)
  : impl_(new WRegExpValidatorImpl(rx))
{ }

WRegExpValidator::WRegExpValidator(const WString& s, WObject *parent)
  : WValidator(parent),
    impl_(new WRegExpValidatorImpl(boost::regex(s.toUTF8().c_str())))
{ }

WRegExpValidator::~WRegExpValidator()
{
  delete impl_;
}

const boost::regex& WRegExpValidator::regExp() const
{
  return impl_->rx;
}

void WRegExpValidator::setRegExp(const WString& s)
{
  impl_->rx = boost::regex(s.toUTF8().c_str());
  repaint();
}

void WRegExpValidator::setNoMatchText(const WString& text)
{
  setInvalidNoMatchText(text);
}

void WRegExpValidator::setInvalidNoMatchText(const WString& text)
{
  impl_->noMatchText_ = text;
  repaint();
}

WString WRegExpValidator::invalidNoMatchText() const
{
  if (!impl_->noMatchText_.empty())
    return impl_->noMatchText_;
  else
    return WString::fromUTF8("Invalid input");
}

WValidator::State WRegExpValidator::validate(WString& input, int& pos)
  const
{
  std::string text = input.toUTF8();

  if (isMandatory()) {
    if (text.empty())
      return InvalidEmpty;
  } else {
    if (text.empty())
      return Valid;
  }

  if (boost::regex_match(text, impl_->rx))
    return Valid;
  else
    return Invalid;
}

std::string WRegExpValidator::javaScriptValidate(const std::string& jsRef) const
{
  std::string js = "function(e,te,tn){if(e.value.length==0)";

  if (isMandatory())
    js += "return {valid:false,message:te};";
  else
    js += "return {valid:true};";

  std::string s = impl_->rx.str();
  Wt::Utils::replace(s, '/', "\\/");

  js += "var r=/^" + s + "$/; return {valid:r.test(e.value),message:tn};}("
    + jsRef + ',' + invalidBlankText().jsStringLiteral() + ','
    + invalidNoMatchText().jsStringLiteral() + ')';

  return js;
}

void WRegExpValidator::createExtConfig(std::ostream& config) const
{
  std::string s = impl_->rx.str();
  Wt::Utils::replace(s, '/', "\\/");

  config << ",regex:/^" << s << "$/";

  if (!impl_->noMatchText_.empty())
    config << ",regexText:" << impl_->noMatchText_.jsStringLiteral();

  WValidator::createExtConfig(config);
}


}
