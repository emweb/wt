/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Utils.h"
#include "DomElement.h"
#include "Wt/WString"

#include <cstdlib>
#include <sstream>

namespace Wt {

  namespace Utils {

std::string terminate(const std::string& s, char c)
{
  if (s.empty() || s[s.length() - 1] != c)
    return s + c;
  else
    return s;
}

std::string& replace(std::string& s, char c, const std::string& r)
{
  std::string::size_type p = 0;

  while ((p = s.find(c, p)) != std::string::npos) {
    s.replace(p, 1, r);
    p += r.length();
  }

  return s;
}

std::string& replace(std::string& s, const std::string& k, const std::string& r)
{
  std::string::size_type p = 0;

  while ((p = s.find(k, p)) != std::string::npos) {
    s.replace(p, k.length(), r);
    p += r.length();
  }

  return s;
}

const std::string& escapeText(std::string& s, bool newLinesToo)
{
  Wt::Utils::replace(s, '&', "&amp;");
  Wt::Utils::replace(s, '<', "&lt;");
  Wt::Utils::replace(s, '>', "&gt;");
  // replace(s, '"', "&quot;");
  // replace(s, '\'', "&apos;");
  if (newLinesToo)
    Wt::Utils::replace(s, '\n', "<br />");

  return s;
}

std::string eraseWord(const std::string& s, const std::string& w)
{
  std::string ss = s;
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

  return ss;
}

std::string addWord(const std::string& s, const std::string& w)
{
  if (s.empty())
    return w;
  else
    return s + ' ' + w;
}

char *itoa(int value, char *result, int base) {
  char* out = result;
  int quotient = value;

  do {
    *out = "0123456789abcdef"[ std::abs( quotient % base ) ];
    ++out;
    quotient /= base;
  } while (quotient);

  if (value < 0 && base == 10) *out++ = '-';
  std::reverse(result, out);
  *out = 0;
  return result;
}

char *pad_itoa(int value, int length, char *result) {
  static const int exp[] = { 1, 10, 100, 1000, 10000, 100000, 100000 };

  result[length] = 0;

  for (int i = 0; i < length; ++i) {
    int b = exp[length - i - 1];
    if (value >= b)
      result[i] = '0' + (value / b) % 10;
    else
      result[i] = '0';
  }

  return result;
}

char *round_str(double d, int digits, char *buf) {
  static const int exp[] = { 1, 10, 100, 1000, 10000, 100000, 1000000 };

  int i = static_cast<int>(d * exp[digits] + (d > 0 ? 0.49 : -0.49));
  itoa(i, buf);
  char *num = buf;

  if (num[0] == '-')
    ++num;
  int len = std::strlen(num);

  if (len <= digits) {
    int shift = digits + 1 - len;
    for (int i = digits + 1; i >= 0; --i) {
      if (i >= shift)
	num[i] = num[i - shift];
      else
	num[i] = '0';
    }
    len = digits + 1;
  }
  int dotPos = std::max(len - digits, 0);
  for (int i = digits + 1; i >= 0; --i)
    num[dotPos + i + 1] = num[dotPos + i];
  num[dotPos] = '.';

  return buf;
}

void unescapeHexTokens(std::string& v)
{
  for (unsigned i = 0; i < (unsigned)std::max(0, (int)v.length() - 2); ++i) {
    if (v[i] == '%') {
      std::string h = v.substr(i + 1, 2);
      char *e = 0;
      int hval = std::strtol(h.c_str(), &e, 16);

      if (*e != 0)
        continue; // not a proper %XX with XX hexadecimal format

      v.replace(i, 3, 1, (char)hval);
    }
  }
}

void urlDecode(std::string &s)
{
  replace(s, '+', " ");
  unescapeHexTokens(s);
}

std::string urlEncode(const std::string& url)
{
  return DomElement::urlEncodeS(url);
}
  }
}
