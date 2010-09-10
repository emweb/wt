/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/lexical_cast.hpp>

#include "Wt/WDoubleValidator"
#include "Wt/WString"

namespace Wt {

WDoubleValidator::WDoubleValidator(WObject *parent)
  : WValidator(parent),
    bottom_(-std::numeric_limits<double>::max()),
    top_(std::numeric_limits<double>::max())
{ }

WDoubleValidator::WDoubleValidator(double bottom, double top, WObject *parent)
  : WValidator(parent),
    bottom_(bottom),
    top_(top)
{ }

void WDoubleValidator::setBottom(double bottom)
{
  if (bottom != bottom_) {
    bottom_ = bottom;
    repaint();
  }
}

void WDoubleValidator::setTop(double top)
{
  if (top != top_) {
    top_ = top;
    repaint();
  }
}

void WDoubleValidator::setRange(double bottom, double top)
{
  setBottom(bottom);
  setTop(top);
}

void WDoubleValidator::setInvalidNotANumberText(const WString& text)
{
  nanText_ = text;
  repaint();
}

WString WDoubleValidator::invalidNotANumberText() const
{
  if (!nanText_.empty())
    return nanText_;
  else
    return WString::tr("Wt.WDoubleValidator.NotANumber");
}

void WDoubleValidator::setInvalidTooSmallText(const WString& text)
{
  tooSmallText_ = text;
  repaint();
}

WString WDoubleValidator::invalidTooSmallText() const
{
  if (!tooSmallText_.empty()) {
    WString s = tooSmallText_;
    s.arg(bottom_).arg(top_);
    return s;
  } else
    if (bottom_ == -std::numeric_limits<double>::max())
      return WString();
    else
      if (top_ == std::numeric_limits<double>::max())
        return WString::tr("Wt.WDoubleValidator.TooSmall").arg(bottom_);
      else
        return WString::tr("Wt.WDoubleValidator.BadRange").
          arg(bottom_).arg(top_);
}

void WDoubleValidator::setInvalidTooLargeText(const WString& text)
{
  tooLargeText_ = text;
  repaint();
}

WString WDoubleValidator::invalidTooLargeText() const
{
  if (!tooLargeText_.empty()) {
    WString s = tooLargeText_;
    s.arg(bottom_).arg(top_);
    return s;
  } else
    if (top_ == std::numeric_limits<double>::max())
      return WString();
    else
      if (bottom_ == -std::numeric_limits<int>::max())
	return WString::tr("Wt.WDoubleValidator.TooLarge").arg(top_);
      else
	return WString::tr("Wt.WDoubleValidator.BadRange").
          arg(bottom_).arg(top_);
}


WValidator::State WDoubleValidator::validate(WT_USTRING& input) const
{
  std::string text = input.toUTF8();

  if (isMandatory()) {
    if (text.empty())
      return InvalidEmpty;
  } else {
    if (text.empty())
      return Valid;
  }

  try {
    double i = boost::lexical_cast<double>(text);

    if ((i >= bottom_) && (i <= top_))
      return Valid;
    else
      return Invalid;
  } catch (boost::bad_lexical_cast& e) {
    return Invalid;
  }
}

std::string WDoubleValidator::javaScriptValidate(const std::string& jsRef) const
{
  std::string js = "function(e,te,tn,ts,tb){if(e.value.length==0)";

  if (isMandatory())
    js += "return {valid:false,message:te};";
  else
    js += "return {valid:true};";

  js += "var n=Number(e.value);"
    "if (isNaN(n)) return {valid:false,message:tn};";

  if (bottom_ != -std::numeric_limits<double>::max())
    js += "if(n<" + boost::lexical_cast<std::string>(bottom_)
      + ") return {valid:false,message:ts};";
  if (top_ != std::numeric_limits<double>::max())
    js += "if(n>" + boost::lexical_cast<std::string>(top_)
      + ") return {valid:false,message:tb};";

  js += "return {valid:true};}(" + jsRef + ','
    + invalidBlankText().jsStringLiteral() + ','
    + invalidNotANumberText().jsStringLiteral() + ','
    + invalidTooSmallText().jsStringLiteral() + ','
    + invalidTooLargeText().jsStringLiteral() + ')';

  return js;
}

#ifndef WT_TARGET_JAVA
void WDoubleValidator::createExtConfig(std::ostream& config) const
{
  if (bottom_ >= 0)
    config << ",allowNegative:false";
  if (bottom_ != -std::numeric_limits<double>::max())
    config << ",minValue:" << bottom_;
  if (top_ != std::numeric_limits<double>::max())
    config << ",maxValue:" << top_;

  if (!tooSmallText_.empty())
    config << ",minText:" << tooSmallText_.jsStringLiteral();
  if (!tooLargeText_.empty())
    config << ",maxText:" << tooLargeText_.jsStringLiteral();
  if (!nanText_.empty())
    config << ",nanText:" << nanText_.jsStringLiteral();

  WValidator::createExtConfig(config);
}
#endif //WT_TARGET_JAVA

}
