/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/algorithm/string.hpp>

#include "Wt/WDoubleValidator.h"
#include "Wt/WString.h"
#include "Wt/WStringStream.h"
#include "Wt/WApplication.h"


#ifndef WT_DEBUG_JS
#include "js/WDoubleValidator.min.js"
#endif

namespace Wt {

WDoubleValidator::WDoubleValidator()
  : bottom_(-std::numeric_limits<double>::max()),
    top_(std::numeric_limits<double>::max()),
    ignoreTrailingSpaces_(false)
{ }

WDoubleValidator::WDoubleValidator(double bottom, double top)
  : bottom_(bottom),
    top_(top),
    ignoreTrailingSpaces_(false)
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

void WDoubleValidator::setIgnoreTrailingSpaces(bool b) {
  if(ignoreTrailingSpaces_ != b)  {
	ignoreTrailingSpaces_ = b;
	repaint();
  }
}

WValidator::Result WDoubleValidator::validate(const WT_USTRING& input) const
{
  if (input.empty())
    return WValidator::validate(input);
  
  std::string text = input.toUTF8();
  
  if(ignoreTrailingSpaces_)
	boost::trim(text);

  try {
    double i = WLocale::currentLocale().toDouble(text);

    if (i < bottom_)
      return Result(ValidationState::Invalid, invalidTooSmallText());
    else if (i > top_)
      return Result(ValidationState::Invalid, invalidTooLargeText());
    else
      return Result(ValidationState::Valid);
  } catch (std::exception& e) {
    return Result(ValidationState::Invalid, invalidNotANumberText());
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
	 << ','
	 << ignoreTrailingSpaces_
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

}
