#include "Wt/WApplication"
#include "Wt/WLocale"

#include "WebUtils.h"
#include <boost/lexical_cast.hpp>

#include <cctype>

namespace Wt {

namespace {
const WLocale systemLocale;
}

WLocale::WLocale()
  : decimalPoint_("."),
    groupSeparator_(""),
    dateFormat_("yyyy-MM-dd"),
    dateTimeFormat_("yyyy-MM-dd HH:mm:ss")
{ }

WLocale::WLocale(const WLocale& other)
  : name_(other.name_),
    decimalPoint_(other.decimalPoint()),
    groupSeparator_(other.groupSeparator()),
    dateFormat_(other.dateFormat()),
    dateTimeFormat_(other.dateTimeFormat()),
    time_zone_(other.time_zone_)
{ }

WLocale::WLocale(const std::string& name)
  : name_(name),
    decimalPoint_(systemLocale.decimalPoint()),
    groupSeparator_(systemLocale.groupSeparator()),
    dateFormat_(systemLocale.dateFormat()),
    dateTimeFormat_(systemLocale.dateTimeFormat()),
    time_zone_(systemLocale.time_zone_)
{ }

WLocale::WLocale(const char *name)
  : name_(name),
    decimalPoint_(systemLocale.decimalPoint()),
    groupSeparator_(systemLocale.groupSeparator()),
    dateFormat_(systemLocale.dateFormat()),
    dateTimeFormat_(systemLocale.dateTimeFormat()),
    time_zone_(systemLocale.time_zone_)
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

void WLocale::setTimeZone(const std::string& posixTimeZone)
{
  time_zone_.reset(new boost::local_time::posix_time_zone(posixTimeZone));
}

std::string WLocale::timeZone() const
{
  if (time_zone_)
    return time_zone_->to_posix_string();
  else
    return std::string();
}

const WLocale& WLocale::currentLocale()
{
  WApplication *app = WApplication::instance();

  if (app)
    return app->locale();
  else
    return systemLocale;
}

bool WLocale::isDefaultNumberLocale() const
{
  return decimalPoint_ == "." && groupSeparator_.empty();
}

double WLocale::toDouble(const WT_USTRING& value) const
{
  if (isDefaultNumberLocale())
    return boost::lexical_cast<double>(value);

  std::string v = value.toUTF8();

  if (!groupSeparator_.empty())
    Utils::replace(v, groupSeparator_, "");
  if (decimalPoint_ != ".")
    Utils::replace(v, decimalPoint_, ".");

  return boost::lexical_cast<double>(v);
}

int WLocale::toInt(const WT_USTRING& value) const
{
  if (groupSeparator_.empty())
    return boost::lexical_cast<int>(value);

  std::string v = value.toUTF8();

  Utils::replace(v, groupSeparator_, "");

  return boost::lexical_cast<int>(v);
}

#ifndef DOXYGEN_ONLY
WT_USTRING WLocale::toString(int value) const
{
  return integerToString(boost::lexical_cast<std::string>(value));
}

WT_USTRING WLocale::toString(unsigned value) const
{
  return integerToString(boost::lexical_cast<std::string>(value));
}

WT_USTRING WLocale::toString(::int64_t value) const
{
  return integerToString(boost::lexical_cast<std::string>(value));
}

WT_USTRING WLocale::toString(::uint64_t value) const
{
  return integerToString(boost::lexical_cast<std::string>(value));
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
  std::string v = boost::lexical_cast<std::string>(value);
  return doubleToString(v);
}

WT_USTRING WLocale::toFixedString(double value, int precision) const
{
  std::stringstream ss;
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
    if (i < decimalPoint - 1 && ((decimalPoint - i - 1) % 3 == 0))
      result += groupSeparator_;
  }

  result += v.substr(decimalPoint);

  return result;
}

}
