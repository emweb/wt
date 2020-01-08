/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/algorithm/string.hpp>

#include "Wt/WApplication.h"
#include "Wt/WIntValidator.h"
#include "Wt/WString.h"
#include "Wt/WStringStream.h"


#ifndef WT_DEBUG_JS
#include "js/WIntValidator.min.js"
#endif

namespace Wt {

WIntValidator::WIntValidator()
  : bottom_(std::numeric_limits<int>::min()),
    top_(std::numeric_limits<int>::max()),
    ignoreTrailingSpaces_(false)
{ }

WIntValidator::WIntValidator(int bottom, int top)
  : bottom_(bottom),
    top_(top),
    ignoreTrailingSpaces_(false)
{ }

void WIntValidator::setBottom(int bottom)
{
  if (bottom != bottom_) {
    bottom_ = bottom;
    repaint();
  }
}

void WIntValidator::setTop(int top)
{
  if (top != top_) {
    top_ = top;
    repaint();
  }
}

void WIntValidator::setRange(int bottom, int top)
{
  setBottom(bottom);
  setTop(top);
}

void WIntValidator::setInvalidNotANumberText(const WString& text)
{
  nanText_ = text;
  repaint();
}

WString WIntValidator::invalidNotANumberText() const
{
  if (!nanText_.empty())
    return nanText_;
  else
    return WString::tr("Wt.WIntValidator.NotAnInteger");
}

void WIntValidator::setInvalidTooSmallText(const WString& text)
{
  tooSmallText_ = text;
  repaint();
}

WString WIntValidator::invalidTooSmallText() const
{
  if (!tooSmallText_.empty()) {
    WString s = tooSmallText_;
    s.arg(bottom_).arg(top_);
    return s;
  } else
    if (bottom_ == std::numeric_limits<int>::min())
      return WString();
    else
      if (top_ == std::numeric_limits<int>::max())
	return WString::tr("Wt.WIntValidator.TooSmall").arg(bottom_);
      else
	return WString::tr("Wt.WIntValidator.BadRange").arg(bottom_).arg(top_);
}

void WIntValidator::setInvalidTooLargeText(const WString& text)
{
  tooLargeText_ = text;
  repaint();
}

WString WIntValidator::invalidTooLargeText() const
{
  if (!tooLargeText_.empty()) {
    WString s = tooLargeText_;
    s.arg(bottom_).arg(top_);
    return s;
  } else
    if (top_ == std::numeric_limits<int>::max())
      return WString();
    else
      if (bottom_ == std::numeric_limits<int>::min())
	return WString::tr("Wt.WIntValidator.TooLarge").arg(top_);
      else
	return WString::tr("Wt.WIntValidator.BadRange").arg(bottom_).arg(top_);
}

void WIntValidator::setIgnoreTrailingSpaces(bool b) {
  if(ignoreTrailingSpaces_ != b)  {
	ignoreTrailingSpaces_ = b;
	repaint();
  }
}


WValidator::Result WIntValidator::validate(const WT_USTRING& input) const
{
  if (input.empty())
    return WValidator::validate(input);

  std::string text = input.toUTF8();

  if (ignoreTrailingSpaces_)
    boost::trim(text);

  try {
    int i = WLocale::currentLocale().toInt(text);

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

void WIntValidator::loadJavaScript(WApplication *app)
{
  LOAD_JAVASCRIPT(app, "js/WIntValidator.js", "WIntValidator", wtjs1);
}

std::string WIntValidator::javaScriptValidate() const
{
  loadJavaScript(WApplication::instance());

  WStringStream js;

  js << "new " WT_CLASS ".WIntValidator("
     << isMandatory()
     << ',';

  if (bottom_ != std::numeric_limits<int>::min())
    js << bottom_;
  else
    js << "null";

  js << ',';

  if (top_ != std::numeric_limits<int>::max())
    js << top_;
  else
    js << "null";

  js << "," << WWebWidget::jsStringLiteral(WLocale::currentLocale()
					   .groupSeparator())
     << ',' << invalidBlankText().jsStringLiteral()
     << ',' << invalidNotANumberText().jsStringLiteral()
     << ',' << invalidTooSmallText().jsStringLiteral()
     << ',' << invalidTooLargeText().jsStringLiteral()
     << ");";

  return js.str();
}

std::string WIntValidator::inputFilter() const
{
  return "[-+0-9]";
}

}
