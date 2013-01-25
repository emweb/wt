/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WApplication"
#include "Wt/WLengthValidator"
#include "Wt/WStringStream"

#ifndef WT_DEBUG_JS
#include "js/WLengthValidator.min.js"
#endif

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
	return WString::tr("Wt.WLengthValidator.TooShort").arg(minLength_);
      else
	return WString::tr("Wt.WLengthValidator.BadRange")
          .arg(minLength_).arg(maxLength_);
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
	return WString::tr("Wt.WLengthValidator.TooLong").arg(maxLength_);
      else
	return WString::tr("Wt.WLengthValidator.BadRange")
          .arg(minLength_).arg(maxLength_);
}

WValidator::Result WLengthValidator::validate(const WT_USTRING& input) const
{
  if (input.empty())
    return WValidator::validate(input);

#ifndef WT_TARGET_JAVA
#ifndef WT_NO_STD_WSTRING
  std::wstring text = input.value();
#else
  std::string text = input.narrow();
#endif
#else
  std::string text = input;
#endif

  if ((int)text.length() < minLength_)
    return Result(Invalid, invalidTooShortText());
  else if ((int)text.length() > maxLength_)
    return Result(Invalid, invalidTooLongText());
  else
    return Result(Valid);
}

void WLengthValidator::loadJavaScript(WApplication *app)
{
  LOAD_JAVASCRIPT(app, "js/WLengthValidator.js", "WLengthValidator", wtjs1);
}

std::string WLengthValidator::javaScriptValidate() const
{
  loadJavaScript(WApplication::instance());

  WStringStream js;

  js << "new " WT_CLASS ".WLengthValidator("
     << isMandatory()
     << ',';

  if (minLength_ != 0)
    js << minLength_;
  else
    js << "null";

  js << ',';

  if (maxLength_ != std::numeric_limits<int>::max())
    js << maxLength_;
  else
    js << "null";

  js << ',' << invalidBlankText().jsStringLiteral()
     << ',' << invalidTooShortText().jsStringLiteral()
     << ',' << invalidTooLongText().jsStringLiteral()
     << ");";

  return js.str();
}

#ifndef WT_TARGET_JAVA
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
#endif //WT_TARGET_JAVA

}
