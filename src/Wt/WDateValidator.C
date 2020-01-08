/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WApplication.h"
#include "Wt/WDateValidator.h"
#include "Wt/WLocale.h"
#include "Wt/WLogger.h"
#include "Wt/WStringStream.h"
#include "Wt/WWebWidget.h"

#ifndef WT_DEBUG_JS
#include "js/WDateValidator.min.js"
#endif

namespace Wt {

LOGGER("WDateValidator");

WDateValidator::WDateValidator()
{
  setFormat(WLocale::currentLocale().dateFormat());
}

WDateValidator::WDateValidator(const WDate& bottom, const WDate& top)
  : bottom_(bottom),
    top_(top)
{
  setFormat(WLocale::currentLocale().dateFormat());
}

WDateValidator::WDateValidator(const WT_USTRING& format)
{
  setFormat(format);
}

WDateValidator::WDateValidator(const WT_USTRING& format,
			       const WDate& bottom, const WDate& top)
  : bottom_(bottom),
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
  if (formats_.empty() || formats_[0] != format) {
    formats_.clear();
    formats_.push_back(format);
    repaint();
  }
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

WValidator::Result WDateValidator::validate(const WT_USTRING& input) const
{
  if (input.empty())
    return WValidator::validate(input);

  for (unsigned i = 0; i < formats_.size(); ++i) {
    try {
      WDate d = WDate::fromString(input, formats_[i]);

      if (d.isValid()) {
	if (!bottom_.isNull())
	  if (d < bottom_)
	    return Result(ValidationState::Invalid, invalidTooEarlyText());

	if (!top_.isNull())
	  if (d > top_)
	    return Result(ValidationState::Invalid, invalidTooLateText());
    
	return Result(ValidationState::Valid);
      }
    } catch (std::exception& e) {
      LOG_WARN("validate(): " << e.what());
    }
  }

  return Result(ValidationState::Invalid, invalidNotADateText());
}

void WDateValidator::loadJavaScript(WApplication *app)
{
  LOAD_JAVASCRIPT(app, "js/WDateValidator.js", "WDateValidator", wtjs1);
}

std::string WDateValidator::javaScriptValidate() const
{
  loadJavaScript(WApplication::instance());

  WStringStream js;

  js << "new " WT_CLASS ".WDateValidator("
     << isMandatory()
     << ",[";

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

}
