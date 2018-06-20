#include "Wt/WApplication.h"
#include "Wt/WLocale.h"

#include "Wt/Date/tz.h"

#include "WebUtils.h"

#include <cctype>

namespace Wt {

namespace {
thread_local WLocale currentLocale_;
const WLocale systemLocale;
}

WLocale::WLocale()
  : decimalPoint_("."),
    groupSeparator_(""),
    dateFormat_("yyyy-MM-dd"),
    timeFormat_("HH:mm:ss"),
    dateTimeFormat_("yyyy-MM-dd HH:mm:ss"),
    timeZone_(nullptr)
{ }

WLocale::WLocale(const WLocale& other)
  : name_(other.name_),
    decimalPoint_(other.decimalPoint()),
    groupSeparator_(other.groupSeparator()),
    dateFormat_(other.dateFormat()),
    timeFormat_(other.timeFormat()),
    dateTimeFormat_(other.dateTimeFormat()),
    timeZone_(other.timeZone())
{ }

WLocale::WLocale(const std::string& name)
  : name_(name),
    decimalPoint_(systemLocale.decimalPoint()),
    groupSeparator_(systemLocale.groupSeparator()),
    dateFormat_(systemLocale.dateFormat()),
    timeFormat_(systemLocale.timeFormat()),
    dateTimeFormat_(systemLocale.dateTimeFormat()),
    timeZone_(systemLocale.timeZone())
{ }

WLocale& WLocale::operator=(const WLocale& other)
{
  name_ = other.name_;
  decimalPoint_ = other.decimalPoint_;
  groupSeparator_ = other.groupSeparator_;
  dateFormat_ = other.dateFormat_;
  timeFormat_ = other.timeFormat_;
  dateTimeFormat_ = other.dateTimeFormat_;
  timeZone_ = other.timeZone();

  return *this;
}

WLocale::WLocale(const char *name)
  : WLocale(std::string(name))
{ }

void WLocale::setDecimalPoint(WT_UCHAR c)
{
  decimalPoint_ = c;
}

void WLocale::setGroupSeparator(WT_UCHAR c)
{
  groupSeparator_ = c;
}

void WLocale::setDateFormat(const WT_USTRING& format)
{
  dateFormat_ = format;
}

void WLocale::setTimeFormat(const WT_USTRING &format)
{
  timeFormat_ = format;
}

void WLocale::setDateTimeFormat(const WT_USTRING& format)
{
  dateTimeFormat_ = format;
}

void WLocale::setTimeZone(const date::time_zone* zone)
{
  timeZone_ = zone;
}

const WLocale& WLocale::currentLocale()
{
  WApplication *app = WApplication::instance();

  if (app)
    return app->locale();
  else
    return currentLocale_;
}

bool WLocale::isDefaultNumberLocale() const
{
  return decimalPoint_ == "." && groupSeparator_.empty();
}

double WLocale::toDouble(const WT_USTRING& value) const
{
  if (isDefaultNumberLocale())
    return Utils::stod(value.toUTF8());

  std::string v = value.toUTF8();

  if (!groupSeparator_.empty())
    Utils::replace(v, groupSeparator_, "");
  if (decimalPoint_ != ".")
    Utils::replace(v, decimalPoint_, ".");

  return Utils::stod(v);
}

int WLocale::toInt(const WT_USTRING& value) const
{
  if (groupSeparator_.empty())
    return Utils::stoi(value.toUTF8());

  std::string v = value.toUTF8();

  Utils::replace(v, groupSeparator_, "");

  return Utils::stoi(v);
}

#ifndef DOXYGEN_ONLY
WT_USTRING WLocale::toString(int value) const
{
  return integerToString(std::to_string(value));
}

WT_USTRING WLocale::toString(unsigned value) const
{
  return integerToString(std::to_string(value));
}

WT_USTRING WLocale::toString(long value) const
{
  return integerToString(std::to_string(value));
}

WT_USTRING WLocale::toString(unsigned long value) const
{
  return integerToString(std::to_string(value));
}

WT_USTRING WLocale::toString(long long value) const
{
  return integerToString(std::to_string(value));
}

WT_USTRING WLocale::toString(unsigned long long value) const
{
  return integerToString(std::to_string(value));
}
#endif // DOXYGEN_ONLY

WT_USTRING WLocale::integerToString(const std::string& v) const
{
  if (groupSeparator_.empty())
    return WT_USTRING::fromUTF8(v);
  else
    return WT_USTRING::fromUTF8(addGrouping(v, v.length()));
}

WT_USTRING WLocale::toString(double value) const
{
  std::stringstream s;
  s.imbue(std::locale::classic());
  s.precision(16);
  s << value;
  return doubleToString(s.str());
}

WT_USTRING WLocale::toFixedString(double value, int precision) const
{
  std::stringstream ss;
  ss.imbue(std::locale::classic());
  ss.precision(precision);
  ss << std::fixed << ((precision > 0) ? std::showpoint : std::noshowpoint) << value;

  return doubleToString(ss.str());
}

WT_USTRING WLocale::doubleToString(std::string v) const
{
  if (isDefaultNumberLocale())
    return WT_USTRING::fromUTF8(v);

  std::size_t dotPos = v.find('.');

  if (dotPos == std::string::npos) {
    if (std::isdigit(v[v.length() - 1]))
      return WT_USTRING::fromUTF8(addGrouping(v, v.length()));
    else
      return WT_USTRING::fromUTF8(v); // Inf, etc...
  } else {
    v.replace(dotPos, 1, decimalPoint_); 
    return WT_USTRING::fromUTF8(addGrouping(v, dotPos));
  }
}

std::string WLocale::addGrouping(const std::string& v, unsigned decimalPoint)
  const 
{
  std::string result;
  result.reserve(v.length() * 2);

  for (unsigned i = 0; i < decimalPoint; ++i) {
    result.push_back(v[i]);
    if (std::isdigit(v[i]) && /* avoid '-,123,456.99' */
	i < decimalPoint - 1 && 
	((decimalPoint - i - 1) % 3 == 0))
      result += groupSeparator_;
  }

  result += v.substr(decimalPoint);

  return result;
}

void WLocale::setCurrentLocale(const WLocale &locale)
{
  WApplication *app = WApplication::instance();
  if (app) {
    app->setLocale(locale);
  }
  else {
    currentLocale_ = locale;
  }
}

}
