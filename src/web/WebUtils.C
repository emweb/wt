/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "WebUtils.h"
#include "DomElement.h"
#include "3rdparty/rapidxml/rapidxml.hpp"
#include "Wt/WException.h"
#include "Wt/WString.h"
#include "Wt/Utils.h"

#include <boost/algorithm/string.hpp>
#include <boost/version.hpp>

#include <ctype.h>
#include <stdio.h>
#include <fstream>
#include <iomanip>
#include <cfloat>

#ifdef WT_WIN32
#include <windows.h>
#define snprintf _snprintf
#else
#include <stdlib.h>
#endif // WIN32

#if !defined(WT_NO_SPIRIT) && BOOST_VERSION >= 104700
#  define SPIRIT_FLOAT_FORMAT
#endif

#ifdef SPIRIT_FLOAT_FORMAT
#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/karma.hpp>
#else
#include <boost/math/special_functions/fpclassify.hpp>
#include <boost/math/special_functions/sign.hpp>
#endif // SPIRIT_FLOAT_FORMAT

#include <boost/spirit/include/qi_parse.hpp>
#include <boost/spirit/include/qi_numeric.hpp>

// Qnx gcc 4.4.2
#ifdef isnan
#undef isnan
#endif
#ifdef isinf
#undef isinf
#endif

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
    Wt::rapidxml::xml_document<>::copy_check_utf8(c, b);
    for (char *i = buf; i < b; ++i)
      sout << *i;
  }
}

std::string eraseWord(const std::string& s, const std::string& w)
{
  std::string::size_type p;
  std::string::size_type pos = 0;

  while ((p = s.find(w, pos)) != std::string::npos) {
    std::string::size_type e = p + w.length();
    if ((p == 0          || s[p-1] == ' ') &&
	(e == s.length() || s[e] == ' ')) {
      std::string ss = s;
      ss.erase(ss.begin() + p, ss.begin() + e);
      if (p > 1)
	ss.erase(ss.begin() + (p - 1));
      else if (e < ss.length())
	ss.erase(ss.begin() + p);

      return ss;
    }

    pos = p + 1;
  }

  return s;
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

#ifdef SPIRIT_FLOAT_FORMAT
namespace {
  using namespace boost::spirit;
  using namespace boost::spirit::karma;

  // adjust rendering for JS flaots
  template <typename T, int Precision>
  struct JavaScriptPolicy : karma::real_policies<T>
  {
    // not 'nan', but 'NaN'
    template <typename CharEncoding, typename Tag, typename OutputIterator>
    static bool nan (OutputIterator& sink, T n, bool force_sign)
    {
      return string_inserter<CharEncoding, Tag>::call(sink, "NaN");
    }

    // not 'inf', but 'Infinity'
    template <typename CharEncoding, typename Tag, typename OutputIterator>
    static bool inf (OutputIterator& sink, T n, bool force_sign)
    {
      return sign_inserter::call(sink, false, (n<0), force_sign) &&
        string_inserter<CharEncoding, Tag>::call(sink, "Infinity");
    }

    static int floatfield(T t) {
      return (t != 0.0) && ((t < 0.001) || (t > 1E8)) ?
	karma::real_policies<T>::fmtflags::scientific :
	karma::real_policies<T>::fmtflags::fixed;
    }

    // 7 significant numbers; about float precision
    static unsigned precision(T) { return Precision; }

  };

  typedef real_generator<double, JavaScriptPolicy<double, 7> >
    KarmaJavaScriptReal;
  typedef real_generator<double, JavaScriptPolicy<double, 15> >
    KarmaJavaScriptDouble;

}

static inline char *generic_double_to_str(double d, int precision, char *buf)
{
  using namespace boost::spirit;
  using namespace boost::spirit::karma;
  char *p = buf;
  if (d != 0) {
      if (fabs(d) < DBL_MIN) {
        std::stringstream ss;
        ss.imbue(std::locale("C"));
        ss << std::setprecision(precision) << d;
        std::string str = ss.str();
        memcpy(p, str.c_str(), str.length());
        p += str.length();
      } else {
        if (precision <= 7)
          generate(p, KarmaJavaScriptReal(), d);
        else
          generate(p, KarmaJavaScriptDouble(), d);
      }
  }  else
    *p++ = '0';
  *p = '\0';
  return buf;
}
#else
static inline char *generic_double_to_str(double d, char *buf)
{
  if (!boost::math::isnan(d)) {
    if (!boost::math::isinf(d)) {
      sprintf(buf, "%.7g", d);
    } else {
      if (d > 0) {
        sprintf(buf, "Infinity");
      } else {
        sprintf(buf, "-Infinity");
      }
    }
  } else {
    sprintf(buf, "NaN");
  }
  return buf;
}
#endif

char *round_css_str(double d, int digits, char *buf)
{
  static const int exp[] = { 1, 10, 100, 1000, 10000, 100000, 1000000 };

  long long i
    = static_cast<long long>(d * exp[digits] + (d > 0 ? 0.49 : -0.49));

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

char *round_js_str(double d, int digits, char *buf) {
#ifdef SPIRIT_FLOAT_FORMAT
  return generic_double_to_str(d, digits, buf);
#else
  return generic_double_to_str(d, buf);
#endif
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

void inplaceUrlDecode(std::string &text)
{
  // Note: there is a Java-too duplicate of this function in Wt/Utils.C
  std::size_t j = 0;

  for (std::size_t i = 0; i < text.length(); ++i) {
    char c = text[i];

    if (c == '+') {
      text[j++] = ' ';
    } else if (c == '%' && i + 2 < text.length()) {
      std::string h = text.substr(i + 1, 2);
      char *e = 0;
      int hval = std::strtol(h.c_str(), &e, 16);

      if (*e == 0) {
	text[j++] = (char)hval;
	i += 2;
      } else {
	// not a proper %XX with XX hexadecimal format
	text[j++] = c;
      }
    } else
      text[j++] = c;
  }

  text.erase(j);
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
  
  std::unique_ptr<char[]> ftext(new char[length + 1]);
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

template<typename ResultType, typename SpiritType>
ResultType convert(const char *fname, const SpiritType &t, const std::string& v)
{
  auto is_space = [](char c) { return c == ' '; };
  auto it = std::find_if_not(v.cbegin(), v.cend(), is_space);
  ResultType result{0};
  bool success =
      it < v.cend() &&
      boost::spirit::qi::parse(it, v.cend(), t, result) &&
      std::all_of(it, v.cend(), is_space);
  if (!success)
    throw std::invalid_argument(std::string(fname) + "() of " + v + " failed");
  return result;
}

long stol(const std::string& v)
{
  return convert<long>("stol", boost::spirit::long_, v);
}

unsigned long stoul(const std::string& v)
{
  return convert<unsigned long>("stoul", boost::spirit::ulong_, v);
}

long long stoll(const std::string& v)
{
  return convert<long long>("stoll", boost::spirit::long_long, v);
}

unsigned long long stoull(const std::string& v)
{
  return convert<unsigned long long>("stoull", boost::spirit::ulong_long, v);
}

int stoi(const std::string& v)
{
  return convert<int>("stoi", boost::spirit::int_, v);
}

double stod(const std::string& v)
{
  return convert<double>("stod", boost::spirit::double_, v);
}

float stof(const std::string& v)
{
  return convert<float>("stof", boost::spirit::float_, v);
}


  }
}
