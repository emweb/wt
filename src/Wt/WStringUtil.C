/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WStringUtil"
#include "rapidxml/rapidxml.hpp"

#include <locale>

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
  for (std::wstring::const_iterator i = s.begin(); i != s.end(); ++i) {
    retval += std::use_facet<std::ctype<wchar_t> >(loc).narrow(*i, '?');
  }
  return retval;
}

std::string toUTF8(const std::wstring& s)
{
  std::string result;
  result.reserve(s.length() * 3);

  char buf[4];
  for (std::wstring::const_iterator i = s.begin(); i != s.end(); ++i) {
    char *end = buf;
    rapidxml::xml_document<>::insert_coded_character<0>(end, *i);
    for (char *b = buf; b != end; ++b)
      result += *b;
  }

  return result;
}

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

	  wchar_t wc = ((unsigned char)s[i]) & 0x0F;
	  for (unsigned j = 1; j < 4; ++j) {
	    wc <<= 6;
	    wc |= ((unsigned char)s[i+j]) & 0x3F;
	  }

	  result += wc;
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

	  wchar_t wc = ((unsigned char)s[i]) & 0x1F;
	  for (unsigned j = 1; j < 3; ++j) {
	    wc <<= 6;
	    wc |= ((unsigned char)s[i+j]) & 0x3F;
	  }

	  result += wc;
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

	  wchar_t wc = ((unsigned char)s[i]) & 0x3F;
	  for (unsigned j = 1; j < 2; ++j) {
	    wc <<= 6;
	    wc |= ((unsigned char)s[i+j]) & 0x3F;
	  }

	  result += wc;
	}
      }
      i += 1;
    }

    if (!legal)
      result += (wchar_t)0xFFFD;
  }

  return result;
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
