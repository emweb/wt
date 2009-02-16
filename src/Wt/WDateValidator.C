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

namespace Wt {

WDateValidator::WDateValidator(WObject *parent)
  : WValidator(parent),
    format_("yyyy-MM-dd")
{ }

WDateValidator::WDateValidator(const WDate& bottom, const WDate& top,
			       WObject *parent)
  : WValidator(parent),
    format_("yyyy-MM-dd"),
    bottom_(bottom),
    top_(top)
{ }

WDateValidator::WDateValidator(const WString& format, WObject *parent)
  : WValidator(parent),
    format_(format)
{ }

WDateValidator::WDateValidator(const WString& format,
			       const WDate& bottom, const WDate& top,
			       WObject *parent)
  : WValidator(parent),
    format_(format),
    bottom_(bottom),
    top_(top)
{ }

void WDateValidator::setInvalidNotADateText(const WString& text)
{
  notADateText_ = text;
}

WString WDateValidator::invalidNotADateText() const
{
  if (!notADateText_.empty()) {
    WString s = notADateText_;
    s.arg(format_);
    return s;
  } else
    return WString::fromUTF8("Must be a date in the format '") + format_ + "'";
}


void WDateValidator::setFormat(const WString& format)
{
  if (format_ != format) {
    format_ = format;
    repaint();
  }
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
    s.arg(bottom_.toString(format_)).arg(top_.toString(format_));
    return s;
  } else
    if (bottom_.isNull())
      return WString();
    else
      if (top_.isNull())
	return WString::fromUTF8("The date must be after ")
	  + bottom_.toString(format_);
      else
	return WString::fromUTF8("The date must be between ")
	  + bottom_.toString(format_)
	  + " and " + top_.toString(format_);
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
    s.arg(bottom_.toString(format_)).arg(top_.toString(format_));
    return s;
  } else
    if (top_.isNull())
      return WString();
    else
      if (bottom_.isNull())
	return WString::fromUTF8("The date must be before ")
	  + top_.toString(format_);
      else
	return WString::fromUTF8("The date must be between ")
	  + bottom_.toString(format_)
	  + " and " + top_.toString(format_);
}

WDate WDateValidator::parse(const WString& input)
{
  return WDate::fromString(input, "yyyy-MM-dd");
}

WValidator::State WDateValidator::validate(WString& input, int& pos) const
{
  if (input.empty())
    if (isMandatory())
      return InvalidEmpty;
    else
      return Valid;

  try {
    WDate d = WDate::fromString(input, format_);

    if (d.isValid()) {
      if (!bottom_.isNull())
	if (d < bottom_)
	  return Invalid;

      if (!top_.isNull())
	if (d > top_)
	  return Invalid;
    
      return Valid;
    } else
      return Invalid;
  } catch (std::exception& e) {
    wApp->log("warn") << "WDateValidator::validate(): " << e.what();
    return Invalid;
  }
}

std::string WDateValidator::javaScriptValidate(const std::string& jsRef) const
{
  std::stringstream js;

  js << "function(e,te,tn,ts,tb){if(e.value.length==0)";

  if (isMandatory())
    js << "return {valid:false,message:te};";
  else
    js << "return {valid:true};";

  std::string dayGetJS = "1", monthGetJS = "1", yearGetJS = "2000";
  std::string r = WDate::formatToRegExp(format_,
					dayGetJS, monthGetJS, yearGetJS);

  js << "var r=/^" << r << "$/;"
    "var results=r.exec(e.value);"
    "if (results==null) return {valid:false,message:tn};"
    "var month=" << monthGetJS << ";"
    "var day=" << dayGetJS << ";"
    "if ((day<=0)||(day>31)||(month<=0)||(month>12))"
    " return {valid:false,message:tn};"
    "var d=new Date(" << yearGetJS << ",month-1,day);"
    "if (d.getDate() != day || "
        "d.getMonth() != month-1 || "
        "d.getFullYear() != " << yearGetJS << ") "
      "return {valid:false,massage:tn};"
    ;

  if (!bottom_.isNull())
    js << "if(d.getTime()<new Date(" << bottom_.year() << ','
       << bottom_.month()-1 << ',' << bottom_.day() << ").getTime())"
      "return {valid:false,message:ts};";
  if (!top_.isNull())
    js << "if(d.getTime()>new Date(" << top_.year() << ','
       << top_.month()-1 << ',' << top_.day() << ").getTime())"
      "return {valid:false,message:tb};";

  js << "return {valid:true};}(" << jsRef << ','
     << invalidBlankText().jsStringLiteral() << ','
     << invalidNotADateText().jsStringLiteral() << ','
     << invalidTooEarlyText().jsStringLiteral() << ','
     << invalidTooLateText().jsStringLiteral() << ')';

  return js.str();
}

void WDateValidator::createExtConfig(std::ostream& config) const
{
  config << ",format:"
	 << WWebWidget::jsStringLiteral(WDate::extFormat(format_), '\'');

  try {
    if (!bottom_.isNull())
      config << ",minValue:" << bottom_.toString(format_).jsStringLiteral();
    if (top_.isNull())
      config << ",maxValue:" << top_.toString(format_).jsStringLiteral();
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

}
