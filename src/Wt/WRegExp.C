/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WRegExp"
#include "Wt/WLogger"

namespace Wt {

#ifndef WT_HAVE_GNU_REGEX
LOGGER("WRegExp");
#endif

WRegExp::WRegExp()
{
#ifdef WT_HAVE_GNU_REGEX
  valid_ = false;
#endif
}

WRegExp::WRegExp(const WT_USTRING& pattern)
{
#ifdef WT_HAVE_GNU_REGEX
  valid_ = false;
#endif
  setPattern(pattern, 0);
}

WRegExp::~WRegExp()
{
#ifdef WT_HAVE_GNU_REGEX
  if (valid_)
    regfree(&rx_);
#endif // WT_HAVE_GNU_REGEX
}

void WRegExp::setPattern(const WT_USTRING& pattern, WFlags<RegExpFlag> flags)
{
  flags_ = flags;

#ifndef WT_HAVE_GNU_REGEX
  boost::regex::flag_type opt = boost::regex::normal;
  if (flags & MatchCaseInsensitive)
    opt |= boost::regex::icase;

  try {
    rx_.assign(pattern.toUTF8(), opt);
  } catch(const std::exception& e) {
    LOG_ERROR("error parsing pattern '" << pattern << "': " << e.what());
  }
#else
  if (valid_)
    regfree(&rx_);
  pattern_ = pattern;

  int opt = REG_EXTENDED;
  if (flags & MatchCaseInsensitive)
    opt |= REG_ICASE;

  valid_ = regcomp(&rx_, pattern.toUTF8().c_str(), opt) == 0;
#endif
}

WT_USTRING WRegExp::pattern() const
{
#ifndef WT_HAVE_GNU_REGEX
  return WT_USTRING::fromUTF8(rx_.str());
#else
  return pattern_;
#endif
}

bool WRegExp::isValid() const
{
#ifndef WT_HAVE_GNU_REGEX
  return rx_.status() == 0;
#else
  return valid_;
#endif
}

bool WRegExp::exactMatch(const WT_USTRING& s) const
{
  if (!isValid())
    return false;

#ifndef WT_HAVE_GNU_REGEX
  return boost::regex_match(s.toUTF8(), rx_);
#else
  return regexec(&rx_, s.toUTF8().c_str(), 0, 0, 0) == 0;
#endif
}

}
