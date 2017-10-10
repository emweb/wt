/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
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

namespace Wt {

WRegExpValidator::WRegExpValidator()
{ }

WRegExpValidator::WRegExpValidator(const WT_USTRING& pattern)
  : regex_(pattern.toUTF8())
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
