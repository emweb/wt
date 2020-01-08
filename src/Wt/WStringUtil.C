/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WLogger.h"
#include "Wt/WStringUtil.h"

#include "3rdparty/rapidxml/rapidxml.hpp"

#include <locale>

#if WCHAR_MAX == 0xFFFF
#define TWO_BYTE_CHAR
#else
#define FOUR_BYTE_CHAR
#endif

namespace Wt {

LOGGER("WString");

namespace {
  static const std::size_t stack_buffer_size = 512;

  template<typename OutStrT>
  OutStrT do_widen(const std::string& s, const std::locale &loc)
  {
    typedef typename OutStrT::value_type OutCharT;

    typedef std::codecvt<wchar_t, char, std::mbstate_t> Cvt;
    
    OutStrT result;
    result.reserve(s.length());
    
    const Cvt& myfacet = std::use_facet<Cvt>(loc);
    Cvt::result myresult;
    std::mbstate_t mystate = std::mbstate_t();
   
    wchar_t stack_buffer[stack_buffer_size + 1];
    const char* next_to_convert = s.c_str();
    const char* const to_convert_end = s.c_str() + s.length();
    
    bool error = false;

    while (next_to_convert != to_convert_end) {
      wchar_t* converted_end = stack_buffer;
      myresult = myfacet.in(mystate, next_to_convert, to_convert_end,
			    next_to_convert,
			    stack_buffer, stack_buffer + stack_buffer_size,
			    converted_end);

      result.append((OutCharT*)stack_buffer, (OutCharT*)converted_end);
      
      if (myresult == Cvt::error) {
	result += '?';
	++ next_to_convert;
	error = true;
      }
    }

    if (error)
      LOG_ERROR("widen(): could not widen string: " << s);

    return result;
  }

  template<typename InStrT>
  std::string do_narrow(const InStrT &s, const std::locale &loc)
  {
    typedef std::codecvt<wchar_t, char, std::mbstate_t> cvt;

    const cvt& myfacet = std::use_facet<cvt>(loc);

    cvt::result myresult;

    const wchar_t *pwstr = (const wchar_t*)(s.c_str());
    const wchar_t *pwend = (const wchar_t*)(s.c_str() + s.length());
    const wchar_t *pwc = pwstr;

    int size = s.length() + 1;

    char *pstr = (char*)std::malloc(size);
    char *pc = pstr;

    std::mbstate_t mystate = std::mbstate_t();
    bool error = false;

    for (;;) {
      myresult = myfacet.out(mystate, pwc, pwend, pwc, pc, pstr + size, pc);

      if (myresult == cvt::ok) {
        break;
      } else {
        if (myresult == cvt::partial || pc >= pstr + size) {
          size += s.length();
          std::size_t sofar = pc - pstr;
          pstr = (char *)std::realloc(pstr, size);
          pc = pstr + sofar;
        }

        if (myresult == cvt::error) {
          *pc++ = '?';
          error = true;
#ifdef TWO_BYTE_CHAR
          if (*pwc >= 0xD800 &&
              *pwc < 0xDC00)
            ++pwc; // skip low surrogate too
          if (pwc == pwend)
            break; // highly unusual
#endif
          ++pwc;
        }
      }
    }

    std::string result(pstr, pc - pstr);

    if (error)
      LOG_WARN("narrow(): loss of detail: " << result);

    std::free(pstr);

    return result;
  }

  template<typename OutStrT, typename InStrT>
  OutStrT utf16_to_utf32(const InStrT &s)
  {
    OutStrT result;
    result.reserve(s.size());
    for (std::size_t i = 0; i < s.size(); ++i) {
      typename InStrT::value_type c = s[i];
      if (c < 0xD800 || c > 0xDFFF)
        result.push_back((typename OutStrT::value_type) c);
      else if (i + 1 < s.size() &&
               s[i] >= 0xD800 && s[i] < 0xDC00 &&
               s[i+1] >= 0xDC00 && s[i+1] <= 0xDFFF) {
        result.push_back((typename OutStrT::value_type)
                         (0x10000 + ((s[i] - 0xD800) << 10) + (s[i+1] - 0xDC00)));
        ++i;
      } else {
        result.push_back((typename OutStrT::value_type) 0xFFFD); // invalid
      }
    }
    return result;
  }

  template<typename OutStrT, typename InStrT>
  OutStrT utf32_to_utf16(const InStrT &s)
  {
    OutStrT result;
    result.reserve(s.size());
    for (std::size_t i = 0; i < s.size(); ++i) {
      typename InStrT::value_type c = s[i];
      if (c < 0x10000) {
        if (c < 0xD800 || c > 0xDFFF)
          result.push_back((typename OutStrT::value_type) c);
        else
          result.push_back((typename OutStrT::value_type) 0xFFFD); // invalid
      } else {
        result.push_back((typename OutStrT::value_type) (((c - 0x10000) >> 10) + 0xD800));
        result.push_back((typename OutStrT::value_type) (((c - 0x10000) & 0x3FF) + 0xDC00));
      }
    }
    return result;
  }

  template<typename OutStrT>
  OutStrT utf8_to_utf32(const std::string &s)
  {
    typedef typename OutStrT::value_type char_type;

    OutStrT result;
    result.reserve(s.length());

    for (unsigned i = 0; i < s.length(); ++i) {
      bool legal = false;
      if ((unsigned char)s[i] <= 0x7F) {
        unsigned char c = s[i];
        if (c == 0x09 || c == 0x0A || c == 0x0D || c >= 0x20) {
          result += (char_type)(c);
          legal = true;
        }
      } else if ((unsigned char)s[i] >= 0xF0) {
        if (i + 3 < s.length()) {
          if ((
               // F0 90-BF 80-BF 80-BF
               (                                    (unsigned char)s[i] == 0xF0)
               && (0x90 <= (unsigned char)s[i+1] && (unsigned char)s[i+1] <= 0xBF)
               && (0x80 <= (unsigned char)s[i+2] && (unsigned char)s[i+2] <= 0xBF)
               && (0x80 <= (unsigned char)s[i+3] && (unsigned char)s[i+3] <= 0xBF)
               ) ||
              (
               // F1-F3 80-BF 80-BF 80-BF
               (   0xF1 <= (unsigned char)s[i]   && (unsigned char)s[i] <= 0xF3)
               && (0x80 <= (unsigned char)s[i+1] && (unsigned char)s[i+1] <= 0xBF)
               && (0x80 <= (unsigned char)s[i+2] && (unsigned char)s[i+2] <= 0xBF)
               && (0x80 <= (unsigned char)s[i+3] && (unsigned char)s[i+3] <= 0xBF)
               )) {
            legal = true;

            uint32_t cp = ((unsigned char)s[i]) & 0x0F;
            for (unsigned j = 1; j < 4; ++j) {
              cp <<= 6;
              cp |= ((unsigned char)s[i+j]) & 0x3F;
            }

            char_type wc = cp;
            if ((uint32_t)wc == cp)
              result += wc;
            else
              legal = false;
          }
        }
        i += 3;
      } else if ((unsigned char)s[i] >= 0xE0) {
        if (i + 2 < s.length()) {
          if ((
               // E0 A0*-BF 80-BF
               (                                    (unsigned char)s[i] == 0xE0)
               && (0xA0 <= (unsigned char)s[i+1] && (unsigned char)s[i+1] <= 0xBF)
               && (0x80 <= (unsigned char)s[i+2] && (unsigned char)s[i+2] <= 0xBF)
               ) ||
              (
               // E1-EF 80-BF 80-BF
               (   0xE1 <= (unsigned char)s[i]   && (unsigned char)s[i] <= 0xF1)
               && (0x80 <= (unsigned char)s[i+1] && (unsigned char)s[i+1] <= 0xBF)
               && (0x80 <= (unsigned char)s[i+2] && (unsigned char)s[i+2] <= 0xBF)
               )) {
            legal = true;

            char_type cp = ((unsigned char)s[i]) & 0x1F;
            for (unsigned j = 1; j < 3; ++j) {
              cp <<= 6;
              cp |= ((unsigned char)s[i+j]) & 0x3F;
            }

            char_type wc = cp;
            if (wc == cp)
              result += wc;
            else
              legal = false;
          }
        }
        i += 2;
      } else if ((unsigned char)s[i] >= 0xC0) {
        if (i + 1 < s.length()) {
          if (
              // C2-DF 80-BF
              (   0xC2 <= (unsigned char)s[i]   && (unsigned char)s[i] <= 0xDF)
              && (0x80 <= (unsigned char)s[i+1] && (unsigned char)s[i+1] <= 0xBF)
              ) {
            legal = true;

            char_type cp = ((unsigned char)s[i]) & 0x3F;
            for (unsigned j = 1; j < 2; ++j) {
              cp <<= 6;
              cp |= ((unsigned char)s[i+j]) & 0x3F;
            }

            char_type wc = cp;
            if (wc == cp)
              result += wc;
            else
              legal = false;
          }
        }
        i += 1;
      }

      if (!legal)
        result += (char_type)0xFFFD;
    }

    return result;
  }

  template<typename InStrT>
  std::string utf32_to_utf8(const InStrT &s)
  {
    std::string result;
    result.reserve(s.length() * 3);

    char buf[4];
    for (typename InStrT::const_iterator i = s.begin(); i != s.end(); ++i) {
      char *end = buf;
      try {
        Wt::rapidxml::xml_document<>::insert_coded_character<0>(end, *i);
        for (char *b = buf; b != end; ++b)
          result += *b;
      } catch (Wt::rapidxml::parse_error& e) {
        LOG_ERROR("toUTF8(): " << e.what());
      }
    }

    return result;
  }
}

std::wstring widen(const std::string& s, const std::locale &loc)
{
  return do_widen<std::wstring>(s, loc);
}

std::u16string toUTF16(const std::string& s, const std::locale &loc)
{
#ifdef TWO_BYTE_CHAR
  return do_widen<std::u16string>(s, loc);
#else
  return utf32_to_utf16<std::u16string>(do_widen<std::u32string>(s, loc));
#endif
}

std::u32string toUTF32(const std::string& s, const std::locale &loc)
{
#ifdef TWO_BYTE_CHAR
  return utf16_to_utf32<std::u32string>(do_widen<std::u16string>(s, loc));
#else
  return do_widen<std::u32string>(s, loc);
#endif
}

std::string narrow(const std::wstring& s, const std::locale &loc)
{
  return do_narrow(s, loc);
}

std::string narrow(const std::u16string& s, const std::locale &loc)
{
#ifdef TWO_BYTE_CHAR
  return do_narrow(s, loc);
#else
  return do_narrow(utf16_to_utf32<std::wstring>(s), loc);
#endif
}

std::string narrow(const std::u32string& s, const std::locale &loc)
{
#ifdef TWO_BYTE_CHAR
  return do_narrow(utf32_to_utf16<std::wstring>(s), loc);
#else
  return do_narrow(s, loc);
#endif
}

std::string toUTF8(const std::wstring& s)
{
#ifdef TWO_BYTE_CHAR
  return utf32_to_utf8(utf16_to_utf32<std::u32string>(s));
#else
  return utf32_to_utf8(s);
#endif
}

std::string toUTF8(const std::u16string& s)
{
  return utf32_to_utf8(utf16_to_utf32<std::u32string>(s));
}

std::string toUTF8(const std::u32string& s)
{
  return utf32_to_utf8(s);
}

std::wstring fromUTF8(const std::string& s)
{
#ifdef TWO_BYTE_CHAR
  return utf32_to_utf16<std::wstring>(utf8_to_utf32<std::u32string>(s));
#else
  return utf8_to_utf32<std::wstring>(s);
#endif
}

std::u16string utf8ToUTF16(const std::string &s)
{
  return utf32_to_utf16<std::u16string>(utf8_to_utf32<std::u32string>(s));
}

std::u32string utf8ToUTF32(const std::string &s)
{
  return utf8_to_utf32<std::u32string>(s);
}

std::string fromUTF8(const std::string& s, const std::locale &loc)
{
  return narrow(fromUTF8(s), loc);
}

std::string toUTF8(const std::string& s, const std::locale &loc)
{
  return toUTF8(widen(s, loc));
}

std::u16string toUTF16(const std::wstring& s)
{
#ifdef TWO_BYTE_CHAR
  return std::u16string((const char16_t*)s.c_str());
#else
  return utf32_to_utf16<std::u16string>(s);
#endif
}

std::u16string toUTF16(const std::u32string& s)
{
  return utf32_to_utf16<std::u16string>(s);
}

std::u32string toUTF32(const std::wstring& s)
{
#ifdef TWO_BYTE_CHAR
  return utf16_to_utf32<std::u32string>(s);
#else
  return std::u32string((const char32_t*)s.c_str());
#endif
}

std::u32string toUTF32(const std::u16string& s)
{
  return utf16_to_utf32<std::u32string>(s);
}

std::wostream& streamUTF8(std::wostream &os, const std::string &s)
{
#ifdef TWO_BYTE_CHAR
  os << utf32_to_utf16<std::wstring>(utf8_to_utf32<std::u32string>(s));
#else
  os << utf8_to_utf32<std::wstring>(s);
#endif
  return os;
}

std::string UTF8Substr(const std::string &s, int begin, int length)
{
  std::string retval;
  // pos, beginPos and endPos refer to byte positions in s
  unsigned pos = 0;
  unsigned beginPos = 0;
  unsigned endPos = -1;

  for(int i = 0; i < begin && pos < s.size(); ++i) {
    unsigned char c = s[pos];
    if ((c & 0x80) == 0x0) pos++;
    else if ((c & 0xe0) == 0xc0) pos += 2;
    else if ((c & 0xf0) == 0xe0) pos += 3;
    else if ((c & 0xf8) == 0xf0) pos += 4;
    else pos++; // invalid!
  }
  beginPos = pos;

  if (length != -1) {
    for(int i = 0; i < length && pos < s.size(); ++i) {
      unsigned char c = s[pos];
      if ((c & 0x80) == 0x0) pos++;
      else if ((c & 0xe0) == 0xc0) pos += 2;
      else if ((c & 0xf0) == 0xe0) pos += 3;
      else if ((c & 0xf8) == 0xf0) pos += 4;
      else pos++; // invalid!
    }
    endPos = pos;
    return s.substr(beginPos, endPos - beginPos);
  } else {
    endPos = -1;
    return s.substr(beginPos, std::string::npos);
  }
}

}
