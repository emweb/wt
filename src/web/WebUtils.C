/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "WebUtils.h"
#include "DomElement.h"
#include "thirdparty/rapidxml/rapidxml.hpp"
#include "Wt/WException.h"
#include "Wt/WString.h"
#include "Wt/WStringUtil.h"
#include "Wt/Utils.h"

#include <boost/algorithm/string.hpp>
#include <boost/version.hpp>

#include <cctype>
#include <cfloat>
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <regex>
#include <unordered_map>

#ifdef WT_WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#define snprintf _snprintf
#else
#include <cstdlib>
#endif // WIN32

#if !defined(WT_NO_SPIRIT) && BOOST_VERSION >= 104700 && (BOOST_VERSION < 107600 || BOOST_VERSION >= 107900)
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

char *utoa(unsigned int value, char* result, int base) {
  char* out = result;
  unsigned int quotient = value;

  do {
    *out =
      "0123456789abcdefghijklmnopqrstuvwxyz"[quotient % base];
    ++out;
    quotient /= base;
  } while (quotient);

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
    static bool nan (OutputIterator& sink, WT_MAYBE_UNUSED T n, WT_MAYBE_UNUSED bool forceSign)
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
      return (t != 0.0) && ((std::abs(t) < 0.001) || (std::abs(t) > 1E8)) ?
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

std::string dataUrlDecode(WT_MAYBE_UNUSED const std::string& url,
                          WT_MAYBE_UNUSED std::vector<unsigned char>& data)
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

namespace {
static const std::unordered_map<wchar_t, wchar_t> asciiReplaceMap = {
  {L'à', L'a'}, {L'á', L'a'}, {L'â', L'a'}, {L'ã', L'a'}, {L'ä', L'a'},
  {L'å', L'a'}, {L'ā', L'a'}, {L'ă', L'a'}, {L'ą', L'a'}, {L'ǎ', L'a'},
  {L'ǻ', L'a'}, {L'ạ', L'a'}, {L'ả', L'a'}, {L'ấ', L'a'}, {L'ầ', L'a'},
  {L'ẩ', L'a'}, {L'ẫ', L'a'}, {L'ậ', L'a'}, {L'ắ', L'a'}, {L'ằ', L'a'},
  {L'ẳ', L'a'}, {L'ẵ', L'a'}, {L'ặ', L'a'},
  {L'è', L'e'}, {L'é', L'e'}, {L'ê', L'e'}, {L'ë', L'e'}, {L'ē', L'e'},
  {L'ĕ', L'e'}, {L'ė', L'e'}, {L'ę', L'e'}, {L'ě', L'e'}, {L'ẹ', L'e'},
  {L'ẻ', L'e'}, {L'ẽ', L'e'}, {L'ế', L'e'}, {L'ề', L'e'}, {L'ể', L'e'},
  {L'ễ', L'e'}, {L'ệ', L'e'},
  {L'ì', L'i'}, {L'í', L'i'}, {L'î', L'i'}, {L'ï', L'i'}, {L'ĩ', L'i'},
  {L'ī', L'i'}, {L'ĭ', L'i'}, {L'į', L'i'}, {L'ǐ', L'i'}, {L'ỉ', L'i'},
  {L'ị', L'i'},
  {L'ò', L'o'}, {L'ó', L'o'}, {L'ô', L'o'}, {L'õ', L'o'}, {L'ö', L'o'},
  {L'ø', L'o'}, {L'ō', L'o'}, {L'ŏ', L'o'}, {L'ő', L'o'}, {L'ǒ', L'o'},
  {L'ǿ', L'o'}, {L'ọ', L'o'}, {L'ỏ', L'o'}, {L'ố', L'o'}, {L'ồ', L'o'},
  {L'ổ', L'o'}, {L'ỗ', L'o'}, {L'ộ', L'o'}, {L'ớ', L'o'}, {L'ờ', L'o'},
  {L'ở', L'o'}, {L'ỡ', L'o'}, {L'ợ', L'o'},
  {L'ù', L'u'}, {L'ú', L'u'}, {L'û', L'u'}, {L'ü', L'u'}, {L'ũ', L'u'},
  {L'ū', L'u'}, {L'ŭ', L'u'}, {L'ů', L'u'}, {L'ű', L'u'}, {L'ų', L'u'},
  {L'ǔ', L'u'}, {L'ụ', L'u'}, {L'ủ', L'u'}, {L'ứ', L'u'}, {L'ừ', L'u'},
  {L'ử', L'u'}, {L'ữ', L'u'}, {L'ự', L'u'},
  {L'ý', L'y'}, {L'ÿ', L'y'}, {L'ŷ', L'y'}, {L'ỳ', L'y'}, {L'ỵ', L'y'},
  {L'ỷ', L'y'}, {L'ỹ', L'y'},

  {L'À', L'A'}, {L'Á', L'A'}, {L'Â', L'A'}, {L'Ã', L'A'}, {L'Ä', L'A'},
  {L'Å', L'A'}, {L'Ā', L'A'}, {L'Ă', L'A'}, {L'Ą', L'A'}, {L'Ǎ', L'A'},
  {L'Ǻ', L'A'}, {L'Ạ', L'A'}, {L'Ả', L'A'}, {L'Ấ', L'A'}, {L'Ầ', L'A'},
  {L'Ẩ', L'A'}, {L'Ẫ', L'A'}, {L'Ậ', L'A'}, {L'Ắ', L'A'}, {L'Ằ', L'A'},
  {L'Ẳ', L'A'}, {L'Ẵ', L'A'}, {L'Ặ', L'A'},
  {L'È', L'E'}, {L'É', L'E'}, {L'Ê', L'E'}, {L'Ë', L'E'}, {L'Ē', L'E'},
  {L'Ĕ', L'E'}, {L'Ė', L'E'}, {L'Ę', L'E'}, {L'Ě', L'E'}, {L'Ẹ', L'E'},
  {L'Ẻ', L'E'}, {L'Ẽ', L'E'}, {L'Ế', L'E'}, {L'Ề', L'E'}, {L'Ể', L'E'},
  {L'Ễ', L'E'}, {L'Ệ', L'E'},
  {L'Ì', L'I'}, {L'Í', L'I'}, {L'Î', L'I'}, {L'Ï', L'I'}, {L'Ĩ', L'I'},
  {L'Ī', L'I'}, {L'Ĭ', L'I'}, {L'Į', L'I'}, {L'Ǐ', L'I'}, {L'Ỉ', L'I'},
  {L'Ị', L'I'},
  {L'Ò', L'O'}, {L'Ó', L'O'}, {L'Ô', L'O'}, {L'Õ', L'O'}, {L'Ö', L'O'},
  {L'Ø', L'O'}, {L'Ō', L'O'}, {L'ŏ', L'O'}, {L'Ő', L'O'}, {L'Ǒ', L'O'},
  {L'Ǿ', L'O'}, {L'Ọ', L'O'}, {L'Ỏ', L'O'}, {L'Ố', L'O'}, {L'Ồ', L'O'},
  {L'Ổ', L'O'}, {L'Ỗ', L'O'}, {L'Ộ', L'O'}, {L'Ớ', L'O'}, {L'Ờ', L'O'},
  {L'Ở', L'O'}, {L'Ỡ', L'O'}, {L'Ợ', L'O'},
  {L'Ù', L'U'}, {L'Ú', L'U'}, {L'Û', L'U'}, {L'Ü', L'U'}, {L'Ũ', L'U'},
  {L'Ū', L'U'}, {L'Ŭ', L'U'}, {L'Ů', L'U'}, {L'Ű', L'U'}, {L'Ų', L'U'},
  {L'Ǔ', L'U'}, {L'Ụ', L'U'}, {L'Ủ', L'U'}, {L'Ứ', L'U'}, {L'Ừ', L'U'},
  {L'Ử', L'U'}, {L'Ữ', L'U'}, {L'Ự', L'U'},
  {L'Ý', L'Y'}, {L'Ÿ', L'Y'}, {L'Ŷ', L'Y'}, {L'Ỳ', L'Y'}, {L'Ỵ', L'Y'},
  {L'Ỷ', L'Y'}, {L'Ỹ', L'Y'},

  {L'ç', L'c'}, {L'ć', L'c'}, {L'ĉ', L'c'}, {L'ċ', L'c'}, {L'č', L'c'},
  {L'Ç', L'C'}, {L'Ć', L'C'}, {L'Ĉ', L'C'}, {L'Ċ', L'C'}, {L'Č', L'C'},
  {L'ď', L'd'}, {L'đ', L'd'}, {L'Ď', L'D'}, {L'Đ', L'D'},
  {L'ĝ', L'g'}, {L'ğ', L'g'}, {L'ġ', L'g'}, {L'ģ', L'g'},
  {L'Ĝ', L'G'}, {L'Ğ', L'G'}, {L'Ġ', L'G'}, {L'Ģ', L'G'},
  {L'ĥ', L'h'}, {L'ħ', L'h'}, {L'Ĥ', L'H'}, {L'Ħ', L'H'},
  {L'ĵ', L'j'}, {L'Ĵ', L'J'},
  {L'ķ', L'k'}, {L'Ķ', L'K'},
  {L'ĺ', L'l'}, {L'ļ', L'l'}, {L'ľ', L'l'}, {L'ŀ', L'l'}, {L'ł', L'l'},
  {L'Ĺ', L'L'}, {L'Ļ', L'L'}, {L'Ľ', L'L'}, {L'Ŀ', L'L'}, {L'Ł', L'L'},
  {L'ñ', L'n'}, {L'ń', L'n'}, {L'ņ', L'n'}, {L'ň', L'n'}, {L'ŉ', L'n'},
  {L'Ñ', L'N'}, {L'Ń', L'N'}, {L'Ņ', L'N'}, {L'Ň', L'N'},
  {L'ŕ', L'r'}, {L'ŗ', L'r'}, {L'ř', L'r'},
  {L'Ŕ', L'R'}, {L'Ŗ', L'R'}, {L'Ř', L'R'},
  {L'ś', L's'}, {L'ŝ', L's'}, {L'ş', L's'}, {L'š', L's'}, {L'ș', L's'},
  {L'Ś', L'S'}, {L'Ŝ', L'S'}, {L'Ş', L'S'}, {L'Š', L'S'}, {L'Ș', L'S'},
  {L'ţ', L't'}, {L'ť', L't'}, {L'ŧ', L't'}, {L'ț', L't'},
  {L'Ţ', L'T'}, {L'Ť', L'T'}, {L'Ŧ', L'T'}, {L'Ț', L'T'},
  {L'ŵ', L'w'}, {L'Ŵ', L'W'},
  {L'ź', L'z'}, {L'ż', L'z'}, {L'ž', L'z'},
  {L'Ź', L'Z'}, {L'Ż', L'Z'}, {L'Ž', L'Z'},

  {L'‘', L'\''}, {L'’', L'\''}, {L'‚', L'\''}, {L'‛', L'\''},
  {L'“', L'"'},  {L'”', L'"'},  {L'„', L'"'},  {L'‟', L'"'},
  {L'‹', L'<'},  {L'›', L'>'},
  {L'«', L'"'},  {L'»', L'"'},
  {L'‐', L'-'},  {L'‑', L'-'},  {L'–', L'-'},  {L'—', L'-'},
  {L'―', L'-'},  {L'⸗', L'-'},
  {L'˜', L'~'},
  {L'·', L'.'},  {L'•', L'.'}
};
}

std::string toAscii(const std::wstring& input) {
  if (input.empty()) {
    return "";
  }

  std::wstring s = input;
  for (auto& c : s) {
    auto it = asciiReplaceMap.find(c);
    if (it != asciiReplaceMap.end()) {
      c = it->second;
    }
}

  // Remove any remaining bytes that are outside the standard 7-bit ASCII range
  std::wregex nonAsciiRegex(L"[^\\x00-\\x7F]");
  return Wt::toUTF8(std::regex_replace(s, nonAsciiRegex, L"?"));
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

void fixSelfClosingTags(Wt::rapidxml::xml_node<> *x_node)
{
  for (Wt::rapidxml::xml_node<> *x_child = x_node->first_node(); x_child;
       x_child = x_child->next_sibling())
    fixSelfClosingTags(x_child);

  if (!x_node->first_node()
      && x_node->value_size() == 0
      && !Wt::DomElement::isSelfClosingTag
      (std::string(x_node->name(), x_node->name_size()))) {
    // We need to add an emtpy data node since <div /> is illegal HTML
    // (but valid XML / XHTML)
    Wt::rapidxml::xml_node<> *empty = x_node->document()->allocate_node(Wt::rapidxml::node_data);
    x_node->append_node(empty);
  }
}

  }
}
