/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <sstream>

#include "Wt/WApplication"
#include "Wt/WDateValidator"
#include "Wt/WLogger"
#include "Wt/WWebWidget"

#include "JavaScriptLoader.h"
#include "EscapeOStream.h"

#ifndef WT_DEBUG_JS
#include "js/WDateValidator.min.js"
#endif

namespace Wt {

WDateValidator::WDateValidator(WObject *parent)
  : WValidator(parent)
{
  setFormat("yyyy-MM-dd");
}

WDateValidator::WDateValidator(const WDate& bottom, const WDate& top,
			       WObject *parent)
  : WValidator(parent),
    bottom_(bottom),
    top_(top)
{
  setFormat("yyyy-MM-dd");
}

WDateValidator::WDateValidator(const WT_USTRING& format, WObject *parent)
  : WValidator(parent)
{
  setFormat(format);
}

WDateValidator::WDateValidator(const WT_USTRING& format,
			       const WDate& bottom, const WDate& top,
			       WObject *parent)
  : WValidator(parent),
    bottom_(bottom),
    top_(top)
{
  setFormat(format);
}

void WDateValidator::setInvalidNotADateText(const WString& text)
{
  notADateText_ = text;
}

WString WDateValidator::invalidNotADateText() const
{
  if (!notADateText_.empty()) {
    WString s = notADateText_;
    s.arg(formats_[0]);
    return s;
  } else
    return WString::tr("Wt.WDateValidator.WrongFormat").arg(formats_[0]);
}


void WDateValidator::setFormat(const WT_USTRING& format)
{
  formats_.clear();
  formats_.push_back(format);
  repaint();
}

void WDateValidator::setFormats(const std::vector<WT_USTRING>& formats)
{
  formats_ = formats;
  repaint();
}

void WDateValidator::setBottom(const WDate& bottom)
{
  if (bottom_ != bottom) {
    bottom_ = bottom;
    repaint();
  }
}

void WDateValidator::setTop(const WDate& top)
{
  if (top_ != top) {
    top_ = top;
    repaint();
  }
}

void WDateValidator::setInvalidTooEarlyText(const WString& text)
{
  tooEarlyText_ = text;
  repaint();
}

WString WDateValidator::invalidTooEarlyText() const
{
  if (!tooEarlyText_.empty()) {
    WString s = tooEarlyText_;
    s.arg(bottom_.toString(formats_[0])).arg(top_.toString(formats_[0]));
    return s;
  } else
    if (bottom_.isNull())
      return WString();
    else
      if (top_.isNull())
        return WString::tr("Wt.WDateValidator.DateTooEarly")
          .arg(bottom_.toString(formats_[0]));
      else
        return WString::tr("Wt.WDateValidator.WrongDateRange")
          .arg(bottom_.toString(formats_[0]))
          .arg(top_.toString(formats_[0]));
}

void WDateValidator::setInvalidTooLateText(const WString& text)
{
  tooLateText_ = text;
  repaint();
}

WString WDateValidator::invalidTooLateText() const
{
  if (!tooLateText_.empty()) {
    WString s = tooLateText_;
    s.arg(bottom_.toString(formats_[0])).arg(top_.toString(formats_[0]));
    return s;
  } else
    if (top_.isNull())
      return WString();
    else
      if (bottom_.isNull())
        return WString::tr("Wt.WDateValidator.DateTooLate")
          .arg(top_.toString(formats_[0]));
      else
        return WString::tr("Wt.WDateValidator.WrongDateRange")
          .arg(bottom_.toString(formats_[0]))
          .arg(top_.toString(formats_[0]));
}

#ifndef WT_DEPRECATED_3_0_0
WDate WDateValidator::parse(const WString& input)
{
  return WDate::fromString(input, "yyyy-MM-dd");
}
#endif // WT_DEPRECATED_3_0_0

WValidator::State WDateValidator::validate(WT_USTRING& input) const
{
  if (input.empty())
    return isMandatory() ? InvalidEmpty : Valid;

  for (unsigned i = 0; i < formats_.size(); ++i) {
    try {
      WDate d = WDate::fromString(input, formats_[i]);

      if (d.isValid()) {
	if (!bottom_.isNull())
	  if (d < bottom_)
	    return Invalid;

	if (!top_.isNull())
	  if (d > top_)
	    return Invalid;
    
	return Valid;
      }
    } catch (std::exception& e) {
      wApp->log("warn") << "WDateValidator::validate(): " << e.what();
    }
  }

  return Invalid;
}

void WDateValidator::loadJavaScript(WApplication *app)
{
  const char *THIS_JS = "js/WDateValidator.js";

  if (!app->javaScriptLoaded(THIS_JS)) {
    LOAD_JAVASCRIPT(app, THIS_JS, "WDateValidator", wtjs1);
    app->setJavaScriptLoaded(THIS_JS);
  }
}


std::string WDateValidator::javaScriptValidate() const
{
  loadJavaScript(WApplication::instance());

  SStream js;

  js << "new " WT_CLASS ".WDateValidator("
     << (isMandatory() ? "true" : "false") << ",[";

  for (unsigned i = 0; i < formats_.size(); ++i) {
    WDate::RegExpInfo r = WDate::formatToRegExp(formats_[i]);

    if (i != 0)
      js << ',';

    js << "{"
       << "regexp:" << WWebWidget::jsStringLiteral(r.regexp) << ','
       << "getMonth:function(results){" << r.monthGetJS << ";},"
       << "getDay:function(results){" << r.dayGetJS << ";},"
       << "getYear:function(results){" << r.yearGetJS << ";}"
       << "}";
  }

  js << "],";

  if (!bottom_.isNull())
    js << "new Date("
       << bottom_.year() << ',' << bottom_.month()-1 << ',' << bottom_.day()
       << ")";
  else
    js << "null";

  js << ',';

  if (!top_.isNull())
    js << "new Date("
       << top_.year() << ',' << top_.month()-1 << ',' << top_.day()
       << ")";
  else
    js << "null";

  js << ',' << invalidBlankText().jsStringLiteral()
     << ',' << invalidNotADateText().jsStringLiteral()
     << ',' << invalidTooEarlyText().jsStringLiteral()
     << ',' << invalidTooLateText().jsStringLiteral()
     << ");";

  return js.str();
}

#ifndef WT_TARGET_JAVA
void WDateValidator::createExtConfig(std::ostream& config) const
{
  config << ",format:"
	 << WWebWidget::jsStringLiteral(WDate::extFormat(formats_[0]), '\'');

  try {
    if (!bottom_.isNull())
      config << ",minValue:"
	     << WWebWidget::jsStringLiteral(bottom_.toString(formats_[0]));
    if (top_.isNull())
      config << ",maxValue:"
	     << WWebWidget::jsStringLiteral(top_.toString(formats_[0]));
  } catch (std::exception& e) {
    wApp->log("error") << "WDateValidator: " << e.what();
  }

  if (!tooEarlyText_.empty())
    config << ",minText:" << tooEarlyText_.jsStringLiteral();
  if (!tooLateText_.empty())
    config << ",maxText:" << tooLateText_.jsStringLiteral();
  if (!notADateText_.empty())
    config << ",invalidText:" << notADateText_.jsStringLiteral();

  WValidator::createExtConfig(config);
}
#endif //WT_TARGET_JAVA

}
