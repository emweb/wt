/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/regex.hpp>

#include "Wt/WApplication"
#include "Wt/WRegExpValidator"
#include "Wt/WRegExp"
#include "Wt/WString"

#include "Utils.h"
#include "JavaScriptLoader.h"
#include "EscapeOStream.h"

#ifndef WT_DEBUG_JS
#include "js/WRegExpValidator.min.js"
#endif

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
    regexp_->setPattern(pattern, regexp_->flags());

  repaint();
}

void WRegExpValidator::setFlags(WFlags<RegExpFlag> flags)
{
  if (!regexp_)
    regexp_ = new WRegExp(".*");

  regexp_->setPattern(regexp_->pattern(), flags);
}

WFlags<RegExpFlag> WRegExpValidator::flags() const
{
  if (regexp_)
    return regexp_->flags();
  else 
    return (int)0;
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
    return WString::tr("Wt.WRegExpValidator.Invalid");
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

void WRegExpValidator::loadJavaScript(WApplication *app)
{
  const char *THIS_JS = "js/WRegExpValidator.js";

  if (!app->javaScriptLoaded(THIS_JS)) {
    LOAD_JAVASCRIPT(app, THIS_JS, "WRegExpValidator", wtjs1);
    app->setJavaScriptLoaded(THIS_JS);
  }
}

std::string WRegExpValidator::javaScriptValidate() const
{
  loadJavaScript(WApplication::instance());

  SStream js;

  js << "new " WT_CLASS ".WRegExpValidator("
     << (isMandatory() ? "true" : "false") << ",";

  if (regexp_) {
    js << WWebWidget::jsStringLiteral(regexp_->pattern())
       << ",'";

#ifndef WT_TARGET_JAVA
    WFlags<RegExpFlag> flags = regexp_->flags();
#else
    int flags = regexp_->flags();
#endif

    if (flags & MatchCaseInsensitive)
      js << 'i';

    js << '\'';
  } else
    js << "null, null";

  js << ',' << WWebWidget::jsStringLiteral(invalidBlankText())
     << ',' << WWebWidget::jsStringLiteral(invalidNoMatchText())
     << ");";

  return js.str();
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
