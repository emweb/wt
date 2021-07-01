/*
 * Copyright (C) 2021 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WApplication.h"
#include "Wt/WDateTimeValidator.h"
#include "Wt/WDate.h"
#include "Wt/WTime.h"
#include "Wt/WLocale.h"
#include "Wt/WLogger.h"
#include "Wt/WStringStream.h"
#include "Wt/WWebWidget.h"


#ifndef WT_DEBUG_JS
#include "js/WDateTimeValidator.min.js"
#endif
namespace Wt {

LOGGER("WDateValidator");

WDateTimeValidator::WDateTimeValidator()
{
  setFormat(WLocale::currentLocale().dateTimeFormat());
}

WDateTimeValidator::WDateTimeValidator(const WDateTime& bottom, const WDateTime& top)
  : bottom_(bottom),
    top_(top)
{
  setFormat(WLocale::currentLocale().dateTimeFormat());
}

WDateTimeValidator::WDateTimeValidator(const WT_USTRING& format)
{
  setFormat(format);
}

WDateTimeValidator::WDateTimeValidator(const WT_USTRING& format,
             const WDateTime& bottom, const WDateTime& top)
  : bottom_(bottom),
    top_(top)
{
  setFormat(format);
}

void WDateTimeValidator::setInvalidNotADateTimeText(const WString& text)
{
  notADateTimeText_ = text;
}

WString WDateTimeValidator::invalidNotADateTimeText() const
{
  if (!notADateTimeText_.empty()) {
    WString s = notADateTimeText_;
    s.arg(formats_[0]);
    return s;
  } else {
    return WString::tr("Wt.WDateTimeValidator.WrongFormat").arg(formats_[0]);
  }
}


void WDateTimeValidator::setFormat(const WT_USTRING& format)
{
  std::string formatStr = format.toUTF8();
  std::size_t timeZoneIndex = formatStr.find("Z");
  if (timeZoneIndex != std::string::npos) {
    formatStr.erase(timeZoneIndex, 1);
    LOG_WARN("setFormat(): contains a timezone, which is omitted");
  }

  if (formats_.empty() || formats_[0] != formatStr) {
    formats_.clear();
    formats_.push_back(formatStr);
    repaint();
  }
}

void WDateTimeValidator::setFormats(const std::vector<WT_USTRING>& formats)
{
  std::vector<WT_USTRING> filteredFormats;

  for (const auto& format : formats) {
    std::string formatStr = format.toUTF8();
    std::size_t timeZoneIndex = formatStr.find("Z");
    if (timeZoneIndex != std::string::npos) {
      formatStr.erase(timeZoneIndex, 1);
      LOG_WARN("setFormats(): contains a timezone, which is omitted");
    }
    filteredFormats.push_back(formatStr);
  }
  formats_ = filteredFormats;
  repaint();
}

void WDateTimeValidator::setBottom(const WDateTime& bottom)
{
  if (bottom_ != bottom) {
    bottom_ = bottom;
    repaint();
  }
}

void WDateTimeValidator::setTop(const WDateTime& top)
{
  if (top_ != top) {
    top_ = top;
    repaint();
  }
}

void WDateTimeValidator::setInvalidTooEarlyText(const WString& text)
{
  tooEarlyText_ = text;
  repaint();
}

WString WDateTimeValidator::invalidTooEarlyText() const
{
  if (!tooEarlyText_.empty()) {
    WString s = tooEarlyText_;
    s.arg(bottom_.toString(formats_[0])).arg(top_.toString(formats_[0]));
    return s;
  } else if (tooEarlyText_.empty() && !bottom_.isNull() && !top_.isNull()) {
    return WString::tr("Wt.WDateTimeValidator.WrongDateTimeRange")
           .arg(bottom_.toString(formats_[0]))
           .arg(top_.toString(formats_[0]));
  } else if (tooEarlyText_.empty() && !bottom_.isNull()) {
    return WString::tr("Wt.WDateTimeValidator.DateTimeTooEarly")
           .arg(bottom_.toString(formats_[0]));
  } else {
    return WString();
  }
}

void WDateTimeValidator::setInvalidTooLateText(const WString& text)
{
  tooLateText_ = text;
  repaint();
}

WString WDateTimeValidator::invalidTooLateText() const
{
  if (!tooLateText_.empty()) {
    WString s = tooLateText_;
    s.arg(bottom_.toString(formats_[0])).arg(top_.toString(formats_[0]));
    return s;
  } else if (tooLateText_.empty() && !top_.isNull() && !bottom_.isNull()) {
    return WString::tr("Wt.WDateTimeValidator.WrongDateTimeRange")
           .arg(bottom_.toString(formats_[0]))
           .arg(top_.toString(formats_[0]));
  } else if (tooLateText_.empty() && !top_.isNull()) {
    return WString::tr("Wt.WDateTimeValidator.DateTimeTooLate")
           .arg(top_.toString(formats_[0]));
  } else {
    return WString();
  }
}

WValidator::Result WDateTimeValidator::validate(const WT_USTRING& input) const
{
  if (input.empty()) {
    return WValidator::validate(input);
  }

  for (unsigned i = 0; i < formats_.size(); ++i) {
    try {
      WDateTime d = WDateTime::fromString(input, formats_[i]);
      if (d.isValid()) {
        if (!bottom_.isNull() && d < bottom_) {
          return Result(ValidationState::Invalid, invalidTooEarlyText());
        }

        if (!top_.isNull() && d > top_) {
          return Result(ValidationState::Invalid, invalidTooLateText());
        }

        return Result(ValidationState::Valid);
      }
    } catch (std::exception& e) {
      LOG_WARN("validate(): " << e.what());
    }
  }

  return Result(ValidationState::Invalid, invalidNotADateTimeText());
}

std::string WDateTimeValidator::javaScriptValidate() const
{
  loadJavaScript(WApplication::instance());

  WStringStream js;

  js << "new " WT_CLASS ".WDateTimeValidator("
     << isMandatory()
     << ",[";

  for (unsigned i = 0; i < formats_.size(); ++i) {
    WDateTime::RegExpInfo dateTimeRegex = WDateTime::formatToRegExp(formats_[i]);

    if (i != 0)
      js << ',';

    js << "{"
       << "regexp:" << WWebWidget::jsStringLiteral(dateTimeRegex.regexp) << ','
       << "getYear:function(results){" << dateTimeRegex.yearGetJS << ";},"
       << "getMonth:function(results){" << dateTimeRegex.monthGetJS << ";},"
       << "getDay:function(results){" << dateTimeRegex.dayGetJS << ";},"
       << "getHour:function(results){" << dateTimeRegex.hourGetJS << ";},"
       << "getMinutes:function(results){" << dateTimeRegex.minuteGetJS << ";},"
       << "getSeconds:function(results){" << dateTimeRegex.secGetJS << ";},"
       << "getMilliseconds:function(results){" << dateTimeRegex.msecGetJS << ";}"
       << "}";
  }

  js << "],";

  if (!bottom_.isNull()) {
    WDate date = bottom_.date();
    WTime time = bottom_.time();
    js << "new Date("
       << date.year() << ',' << date.month()-1 << ',' << date.day() << ','
       << time.hour() << ',' << time.minute() << ',' << time.second() << ',' << time.msec()
       << ")";
  } else {
    js << "null";
  }

  js << ',';

  if (!top_.isNull()) {
    WDate date = top_.date();
    WTime time = top_.time();
    js << "new Date("
       << date.year() << ',' << date.month()-1 << ',' << date.day() << ','
       << time.hour() << ',' << time.minute() << ',' << time.second() << ',' << time.msec()
       << ")";
  } else {
    js << "null";
  }

  js << ',' << invalidBlankText().jsStringLiteral()
     << ',' << invalidNotADateTimeText().jsStringLiteral()
     << ',' << invalidTooEarlyText().jsStringLiteral()
     << ',' << invalidTooLateText().jsStringLiteral()
     << ");";

  return js.str();
}

void WDateTimeValidator::loadJavaScript(WApplication *app)
{
  LOAD_JAVASCRIPT(app, "js/WDateTimeValidator.js", "WDateTimeValidator", wtjs1);
}

}
