/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/regex.hpp>

#include "Wt/WRegExpValidator"
#include "Wt/WRegExp"
#include "Wt/WString"

#include "Utils.h"

namespace Wt {

WRegExpValidator::WRegExpValidator(WObject *parent)
  : WValidator(parent),
    regexp_(0)
{ }

WRegExpValidator::WRegExpValidator(const WT_USTRING& pattern, WObject *parent)
  : WValidator(parent),
    regexp_(new WRegExp(pattern))
{ }

WRegExpValidator::~WRegExpValidator()
{
  delete regexp_;
}

void WRegExpValidator::setRegExp(const WT_USTRING& pattern)
{
  if (!regexp_)
    regexp_ = new WRegExp(pattern);
  else
    regexp_->setPattern(pattern);
  repaint();
}

WT_USTRING WRegExpValidator::regExp() const
{
  return regexp_ ? regexp_->pattern() : WT_USTRING();
}

void WRegExpValidator::setNoMatchText(const WString& text)
{
  setInvalidNoMatchText(text);
}

void WRegExpValidator::setInvalidNoMatchText(const WString& text)
{
  noMatchText_ = text;
  repaint();
}

WString WRegExpValidator::invalidNoMatchText() const
{
  if (!noMatchText_.empty())
    return noMatchText_;
  else
    return WString::fromUTF8("Invalid input");
}

WValidator::State WRegExpValidator::validate(WT_USTRING& input) const
{
  if (isMandatory()) {
    if (input.empty())
      return InvalidEmpty;
  } else {
    if (input.empty())
      return Valid;
  }

  if (!regexp_ || regexp_->exactMatch(input))
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

  if (regexp_) {
    std::string s = regexp_->pattern().toUTF8();
    Wt::Utils::replace(s, '/', "\\/");

    js += "var r=/^" + s + "$/; return {valid:r.test(e.value),message:tn};";
  } else
    js += "return {valid:true};";

  js += "}(" + jsRef + ',' + invalidBlankText().jsStringLiteral() + ','
    + invalidNoMatchText().jsStringLiteral() + ')';

  return js;
}

#ifndef WT_TARGET_JAVA
void WRegExpValidator::createExtConfig(std::ostream& config) const
{
  std::string s = regexp_ ? regexp_->pattern().toUTF8() : "";
  Wt::Utils::replace(s, '/', "\\/");

  config << ",regex:/^" << s << "$/";

  if (!noMatchText_.empty())
    config << ",regexText:" << noMatchText_.jsStringLiteral();

  WValidator::createExtConfig(config);
}
#endif //WT_TARGET_JAVA


}
