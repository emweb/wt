/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WStringUtil"

#include <locale>
#include <boost/program_options/parsers.hpp>

namespace Wt {
std::wstring widen(const std::string& s, const std::locale &loc)
{
  std::wstring retval;
  retval.reserve(s.length());
  for (std::string::const_iterator i = s.begin(); i != s.end(); ++i)
  {
    retval += std::use_facet<std::ctype<wchar_t> >(loc).widen(*i);
  }
  return retval;
}

std::string narrow(const std::wstring& s, const std::locale &loc)
{
  std::string retval;
  retval.reserve(s.length());
  for (std::wstring::const_iterator i = s.begin(); i != s.end(); ++i)
  {
    retval += std::use_facet<std::ctype<wchar_t> >(loc).narrow(*i, '?');
  }
  return retval;
}

std::string toUTF8(const std::wstring& s)
{
  return boost::to_utf8(s);
}

std::wstring fromUTF8(const std::string& s)
{
  return boost::from_utf8(s);
}

std::string fromUTF8(const std::string& s, const std::locale &loc)
{
  return narrow(fromUTF8(s), loc);
}

std::string toUTF8(const std::string& s, const std::locale &loc)
{
  return toUTF8(widen(s, loc));
}

}
