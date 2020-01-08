/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WApplication.h"
#include "Wt/WRegExpValidator.h"
#include "Wt/WString.h"
#include "Wt/WStringStream.h"

#include "WebUtils.h"

#ifndef WT_DEBUG_JS
#include "js/WRegExpValidator.min.js"
#endif

namespace {
#ifndef WT_TARGET_JAVA
  static Wt::RegExpFlag MatchCaseInsensitive = Wt::RegExpFlag::MatchCaseInsensitive;
#else // WT_TARGET_JAVA
  static int MatchCaseInsensitive = 1;
#endif // WT_TARGET_JAVA
}

namespace Wt {

WRegExpValidator::WRegExpValidator()
  : pattern_(),
    regex_(),
    noMatchText_()
{ }

WRegExpValidator::WRegExpValidator(const WT_USTRING& pattern)
  : pattern_(pattern),
    regex_(pattern.toUTF8()),
    noMatchText_()
{ }

WRegExpValidator::~WRegExpValidator()
{ }

void WRegExpValidator::setRegExp(const WT_USTRING& pattern)
{
  regex_.assign(pattern.toUTF8());
  pattern_ = pattern;
  repaint();
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
    return WString::tr("Wt.WRegExpValidator.Invalid");
}

WValidator::Result WRegExpValidator::validate(const WT_USTRING& input) const
{
  if (input.empty())
    return WValidator::validate(input);

  if (std::regex_match(input.toUTF8(), regex_))
    return Result(ValidationState::Valid);
  else
    return Result(ValidationState::Invalid, invalidNoMatchText());
}

void WRegExpValidator::loadJavaScript(WApplication *app)
{
  LOAD_JAVASCRIPT(app, "js/WRegExpValidator.js", "WRegExpValidator", wtjs1);
}

void WRegExpValidator::setFlags(WFlags<RegExpFlag> flags)
{
  if (flags.value() == this->flags().value())
    return;

  if (flags.value() & static_cast<int>(MatchCaseInsensitive))
    regex_.assign(pattern_.toUTF8(), std::regex::icase);
  else
    regex_.assign(pattern_.toUTF8());

  repaint();
}

WFlags<RegExpFlag> WRegExpValidator::flags() const
{
  if (regex_.flags() & std::regex::icase)
    return MatchCaseInsensitive;
  else {
#ifndef WT_TARGET_JAVA
    return WFlags<RegExpFlag>();
#else
    return (int)0;
#endif
  }
}

std::string WRegExpValidator::javaScriptValidate() const
{
  loadJavaScript(WApplication::instance());

  WStringStream js;

  js << "new " WT_CLASS ".WRegExpValidator("
     << isMandatory()
     << ',';

  js << WWebWidget::jsStringLiteral(pattern_)
     << ",'";

  if (regex_.flags() & std::regex::icase)
    js << 'i';

  js << '\'';

  js << ',' << WWebWidget::jsStringLiteral(invalidBlankText())
     << ',' << WWebWidget::jsStringLiteral(invalidNoMatchText())
     << ");";

  return js.str();
}

}
