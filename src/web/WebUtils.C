/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "WebUtils.h"
#include "DomElement.h"
#include "rapidxml/rapidxml.hpp"
#include "Wt/WException"
#include "Wt/WString"
#include "Wt/Utils"

#include <boost/algorithm/string.hpp>

#include <ctype.h>
#include <stdio.h>
#include <fstream>

#ifdef WIN32
#include <windows.h>
#define snprintf _snprintf
#else
#include <stdlib.h>
#endif // WIN32

namespace Wt {
  namespace Utils {

std::string append(const std::string& s, char c)
{
  if (s.empty() || s[s.length() - 1] != c)
    return s + c;
  else
    return s;
}

std::string prepend(const std::string& s, char c)
{
  if (s.empty() || s[0] != c)
    return c + s;
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

std::string lowerCase(const std::string& s)
{
  std::string result = s;
  for (unsigned i = 0; i < result.length(); ++i)
    result[i] = tolower(result[i]);
  return result;
}

void sanitizeUnicode(EscapeOStream& sout, const std::string& text)
{
  char buf[4];

  for (const char *c = text.c_str(); *c;) {
    char *b = buf;
    // but copy_check_utf8() does not declare the following ranges illegal:
    //  U+D800-U+DFFF
    //  U+FFFE-U+FFFF
    rapidxml::xml_document<>::copy_check_utf8(c, b);
    for (char *i = buf; i < b; ++i)
      sout << *i;
  }
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

  if (quotient < 0)
    quotient = -quotient;

  do {
    *out =
      "0123456789abcdefghijklmnopqrstuvwxyz"[quotient % base];
    ++out;
    quotient /= base;
  } while (quotient);

  if (value < 0 && base == 10)
    *out++ = '-';

  std::reverse(result, out);
  *out = 0;

  return result;
}

char *lltoa(long long value, char *result, int base) {
  char* out = result;
  long long quotient = value;

  if (quotient < 0)
    quotient = -quotient;

  do {
    *out =
      "0123456789abcdefghijklmnopqrstuvwxyz"[ quotient % base ];
    ++out;
    quotient /= base;
  } while (quotient);

  if (value < 0 && base == 10)
    *out++ = '-';
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

  long long i = static_cast<long long>(d * exp[digits] + (d > 0 ? 0.49 : -0.49));
  lltoa(i, buf);
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
  int dotPos = (std::max)(len - digits, 0);
  for (int i = digits + 1; i >= 0; --i)
    num[dotPos + i + 1] = num[dotPos + i];
  num[dotPos] = '.';

  return buf;
}

std::string urlEncode(const std::string& url, const std::string& allowed)
{
  return DomElement::urlEncodeS(url, allowed);
}

std::string dataUrlDecode(const std::string& url,
			  std::vector<unsigned char> &data)
{
  return std::string();
}

void split(SplitSet& tokens, const std::string &in, const char *sep,
	   bool compress_adjacent_tokens)
{
    boost::split(tokens, in, boost::is_any_of(sep),
		 compress_adjacent_tokens?
		 boost::algorithm::token_compress_on:
		 boost::algorithm::token_compress_off);
}

std::string EncodeHttpHeaderField(const std::string &fieldname,
                                  const WString &fieldValue)
{
  // This implements RFC 5987
  return fieldname + "*=UTF-8''" + urlEncode(fieldValue.toUTF8());
}

std::string readFile(const std::string& fname) 
{
  std::ifstream f(fname.c_str(), std::ios::in | std::ios::binary);
  
  if (!f)
    throw WException("Could not load " + fname);
  
  f.seekg(0, std::ios::end);
  int length = f.tellg();
  f.seekg(0, std::ios::beg);
  
  boost::scoped_array<char> ftext(new char[length + 1]);
  f.read(ftext.get(), length);
  ftext[length] = 0;

  return std::string(ftext.get());
}

WString formatFloat(const WString &format, double value)
{
  std::string f = format.toUTF8();
  int buflen = f.length() + 15;

  char *buf = new char[buflen];

  snprintf(buf, buflen, f.c_str(), value);
  buf[buflen - 1] = 0;

  WString result = WT_USTRING::fromUTF8(buf);

  delete[] buf;

  return result;

}
  
  }
}
