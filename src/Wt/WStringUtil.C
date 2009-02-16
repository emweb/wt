/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WStringUtil"

#include <locale>
#include <boost/program_options/parsers.hpp>

namespace Wt {

std::wstring widen(const std::string& s)
{
  return std::wstring(s.begin(), s.end());
}

std::string narrow(const std::wstring& s)
{
  return std::string(s.begin(), s.end());
}

std::string toUTF8(const std::wstring& s)
{
  return boost::to_utf8(s);
}

std::wstring fromUTF8(const std::string& s)
{
  return boost::from_utf8(s);
}

}
