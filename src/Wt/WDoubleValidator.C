/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/lexical_cast.hpp>

#include "Wt/WDoubleValidator"
#include "Wt/WString"
#include "Wt/WStringStream"
#include "Wt/WApplication"

#ifndef WT_DEBUG_JS
#include "js/WDoubleValidator.min.js"
#endif

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
      if (bottom_ == -std::numeric_limits<double>::max())
        return WString::tr("Wt.WDoubleValidator.TooLarge").arg(top_);
      else
        return WString::tr("Wt.WDoubleValidator.BadRange").
          arg(bottom_).arg(top_);
}


WValidator::Result WDoubleValidator::validate(const WT_USTRING& input) const
{
  if (input.empty())
    return WValidator::validate(input);

  try {
    double i = WLocale::currentLocale().toDouble(input);

    if (i < bottom_)
      return Result(Invalid, invalidTooSmallText());
    else if (i > top_)
      return Result(Invalid, invalidTooLargeText());
    else
      return Result(Valid);
  } catch (boost::bad_lexical_cast& e) {
    return Result(Invalid, invalidNotANumberText());
  }
}

void WDoubleValidator::loadJavaScript(WApplication *app)
{
  LOAD_JAVASCRIPT(app, "js/WDoubleValidator.js", "WDoubleValidator", wtjs1);
}

std::string WDoubleValidator::javaScriptValidate() const
{
  loadJavaScript(WApplication::instance());

  WStringStream js;

  js << "new " WT_CLASS ".WDoubleValidator("
     << isMandatory()
     << ',';

  if (bottom_ != -std::numeric_limits<double>::max() &&
      bottom_ != -std::numeric_limits<double>::infinity())
    js << bottom_;
  else
    js << "null";

  js << ',';

  if (top_ != std::numeric_limits<double>::max() &&
      top_ != std::numeric_limits<double>::infinity())
    js << top_;
  else
    js << "null";

  js << "," << WWebWidget::jsStringLiteral(WLocale::currentLocale()
					   .decimalPoint())
     << "," << WWebWidget::jsStringLiteral(WLocale::currentLocale()
					   .groupSeparator())
     << ',' << invalidBlankText().jsStringLiteral()
     << ',' << invalidNotANumberText().jsStringLiteral()
     << ',' << invalidTooSmallText().jsStringLiteral()
     << ',' << invalidTooLargeText().jsStringLiteral()
     << ");";

  return js.str();
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
