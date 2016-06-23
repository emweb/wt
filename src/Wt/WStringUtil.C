/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WLogger"
#include "Wt/WStringUtil"

#include "3rdparty/rapidxml/rapidxml.hpp"

#ifndef WT_NO_STD_LOCALE
#include <locale>
#endif

namespace Wt {

LOGGER("WString");

namespace {
  static const std::size_t stack_buffer_size = 512;
}

#ifndef WT_NO_STD_WSTRING
#ifndef WT_NO_STD_LOCALE
std::wstring widen(const std::string& s, const std::locale &loc)
{
  typedef std::codecvt<wchar_t, char, std::mbstate_t> Cvt;
  
  std::wstring result;
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

    result.append(stack_buffer, converted_end);
    
    if (myresult == Cvt::error) {
      result += L'?';
      ++ next_to_convert;
      error = true;
    }
  }

  if (error)
    LOG_ERROR("widen(): could not widen string: " << s);

  return result;
}
#else
// Assumes pure ASCII-7 encoding. The wstring will be UCS encoded.
std::wstring widen(const std::string& s)
{
  std::wstring retval;
  retval.reserve(s.size());
  
  for(unsigned int i = 0; i < s.size(); ++i) {
    if (s[i] & 0x80)
      retval.push_back('?'); // invalid ASCII character
    else
      retval.push_back(s[i]); // ASCII-7 doesn't change in unicode
  }
  return retval;
}
#endif
#endif

#ifndef WT_NO_STD_WSTRING
#ifndef WT_NO_STD_LOCALE
std::string narrow(const std::wstring& s, const std::locale &loc)
{
  typedef std::codecvt<wchar_t, char, std::mbstate_t> Cvt;

  const Cvt& myfacet = std::use_facet<Cvt>(loc);

  Cvt::result myresult;

  const wchar_t *pwstr = s.c_str();
  const wchar_t *pwend = s.c_str() + s.length();
  const wchar_t *pwc = pwstr;

  int size = s.length() + 1;

  char *pstr = new char [size];
  char *pc = pstr;

  std::mbstate_t mystate = std::mbstate_t();
  bool error = false;

  for (;;) {
    myresult = myfacet.out(mystate, pwc, pwend, pwc, pc, pc + size, pc);

    if (myresult == Cvt::ok) {
      break;
    } else {
      if (myresult == Cvt::partial || pc >= pstr + size) {
	size += s.length();
	std::size_t sofar = pc - pstr;
	pstr = (char *)std::realloc(pstr, size);
	pc = pstr + sofar;
      }

      if (myresult == Cvt::error) {
	*pc++ = '?';
	pwc++;
	error = true;
      }
    }
  }

  std::string result(pstr, pc - pstr);

  if (error)
    LOG_WARN("narrow(): loss of detail: " << result);

  delete[] pstr;

  return result;
}
#else
std::string narrow(const std::wstring& s)
{
  std::string retval;
  retval.reserve(s.size());
  
  for (unsigned int i = 0; i < s.size(); ++i) {
    if (retval[i] < 128)
      retval.push_back(s[i]);
    else
      retval.push_back('?');
  }
  return retval;
}
#endif
#endif

#ifndef WT_NO_STD_WSTRING
std::string toUTF8(const std::wstring& s)
{
  std::string result;
  result.reserve(s.length() * 3);

  char buf[4];
  for (std::wstring::const_iterator i = s.begin(); i != s.end(); ++i) {
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
#endif

#ifndef WT_NO_STD_WSTRING
std::wstring fromUTF8(const std::string& s)
{
  std::wstring result;
  result.reserve(s.length());

  for (unsigned i = 0; i < s.length(); ++i) {
    bool legal = false;
    if ((unsigned char)s[i] <= 0x7F) {
      unsigned char c = s[i];
      if (c == 0x09 || c == 0x0A || c == 0x0D || c >= 0x20) {
	result += (wchar_t)(c);
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

          wchar_t wc = cp;
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

	  wchar_t cp = ((unsigned char)s[i]) & 0x1F;
	  for (unsigned j = 1; j < 3; ++j) {
	    cp <<= 6;
	    cp |= ((unsigned char)s[i+j]) & 0x3F;
	  }

          wchar_t wc = cp;
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

	  wchar_t cp = ((unsigned char)s[i]) & 0x3F;
	  for (unsigned j = 1; j < 2; ++j) {
	    cp <<= 6;
	    cp |= ((unsigned char)s[i+j]) & 0x3F;
	  }

          wchar_t wc = cp;
          if (wc == cp)
            result += wc;
          else
            legal = false;
	}
      }
      i += 1;
    }

    if (!legal)
      result += (wchar_t)0xFFFD;
  }

  return result;
}
#endif

#ifndef WT_NO_STD_LOCALE
std::string fromUTF8(const std::string& s, const std::locale &loc)
{
  return narrow(fromUTF8(s), loc);
}
#else
std::string fromUTF8(const std::string& s, CharEncoding encoding)
{
  switch(encoding) {
#ifndef WT_NO_STD_WSTRING
    case DefaultEncoding:
    case LocalEncoding:
      return narrow(fromUTF8(s));
#else
    case DefaultEncoding:
    case LocalEncoding:
      {
        // You may want to rewrite this for your system
        // Eliminate all non-ascii chars
        // TODO: handle multi-byte UTF-8 characters
        std::string retval = s;
        for(std::size_t i = 0; i < retval.size(); ++i)
          if (retval[i] > 127) retval[i] = '?';
        return s;
      }
#endif
    case UTF8: return s;
  }
}
#endif

#ifndef WT_NO_STD_LOCALE
std::string toUTF8(const std::string& s, const std::locale &loc)
{
  return toUTF8(widen(s, loc));
}
#else
std::string toUTF8(const std::string& s)
{
#ifndef WT_NO_STD_WSTRING
  return toUTF8(widen(s));
#else
  // You may want to rewrite this for your system
  std::string retval = s;
  for(std::size_t i = 0; i < retval.size(); ++i)
    if (retval[i] > 127) retval[i] = '?';
  return s;
#endif
}
#endif

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
