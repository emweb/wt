/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Utils.h"
#include "Wt/WString"

namespace Wt {
  namespace Utils {

std::string terminate(const std::string& s, char c)
{
  if (s.empty() || s[s.length() - 1] != c)
    return s + c;
  else
    return s;
}

void replace(std::string& s, char c, const std::string& r)
{
  std::string::size_type p = 0;

  while ((p = s.find(c, p)) != std::string::npos) {
    s.replace(p, 1, r);
    p += r.length();
  }
}

void replace(std::string& s, const std::string& k, const std::string& r)
{
  std::string::size_type p = 0;

  while ((p = s.find(k, p)) != std::string::npos) {
    s.replace(p, k.length(), r);
    p += r.length();
  }
}

WString eraseWord(const WString& s, const std::string& w)
{
  std::string ss = s.toUTF8();
  std::string::size_type p;

  if ((p = ss.find(w)) != std::string::npos) {
    ss.erase(ss.begin() + p, ss.begin() + p + w.length());
    if (p > 1) {
      if (ss[p-1] == ' ')
	ss.erase(ss.begin() + (p - 1));
    } else
      if (p < ss.length() && ss[p] == ' ')
	ss.erase(ss.begin() + p);
  }

  return WString::fromUTF8(ss);
}

WString addWord(const WString& s, const std::string& w)
{
  if (s.empty())
    return WString::fromUTF8(w);
  else
    return s + WString::fromUTF8(' ' + w);
}

  }
}
