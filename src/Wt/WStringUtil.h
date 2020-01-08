// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef WSTRING_UTIL_H_
#define WSTRING_UTIL_H_

#include <string>
#include <Wt/WDllDefs.h>
#include <locale>

namespace Wt {
  /*! \brief Convert a narrow to a wide string.
   *
   * Convert a narrow string to a wide string. This method will interpret
   * the input string as being encoded in the given locale (by default the
   * currently configured global C++ locale).
   *
   * \sa narrow(const std::wstring&, const std::locale &), fromUTF8(const std::string& s)
   *
   * \relates WString
   */
  extern WT_API std::wstring widen(const std::string& s,
      const std::locale &loc = std::locale());

#ifndef WT_TARGET_JAVA
  /*! \brief Convert a narrow string to UTF-16.
   *
   * Convert a narrow string to UTF-16. This method will interpret
   * the input string as being encoded in the given locale (by default the
   * currently configured global C++ locale).
   *
   * \sa narrow(const std::u16string&, const std::locale &), utf8ToUTF16(const std::string &s)
   *
   * \relates WString
   */
  extern WT_API std::u16string toUTF16(const std::string& s,
                                       const std::locale& loc = std::locale());

  /*! \brief Convert a narrow string to UTF-32.
   *
   * Convert a narrow string to UTF-32. This method will interpret
   * the input string as being encoded in the given locale (by default the
   * currently configured global C++ locale).
   *
   * \sa narrow(const std::u32string&, const std::locale &), utf8ToUTF32(const std::string &s)
   *
   * \relates WString
   */
  extern WT_API std::u32string toUTF32(const std::string& s,
                                       const std::locale& loc = std::locale());
#endif // WT_TARGET_JAVA

  /*! \brief Convert a wide to a narrow string.
   *
   * Convert a wide string to a narrow string. This method will encode
   * the characters in the given locale, if possible.
   *
   * In general this will lead to a loss of information. If you wish to
   * preserve all information, you should use toUTF8() instead.
   *
   * \sa widen(const std::string&, const std::locale&), toUTF8(const std::wstring& s)
   *
   * \relates WString
   */
  extern WT_API std::string narrow(const std::wstring& s,
      const std::locale &loc = std::locale());

#ifndef WT_TARGET_JAVA
  /*! \brief Convert a UTF-16 to a narrow string.
   *
   * Convert a UTF-16 string to a narrow string. This method will encode
   * the characters in the given locale, if possible.
   *
   * In general this will lead to a loss of information. If you wish to
   * preserve all information, you should use toUTF8() instead.
   *
   * \sa toUTF16(const std::string&, const std::locale&), toUTF8(const std::u16string& s)
   *
   * \relates WString
   */
  extern WT_API std::string narrow(const std::u16string& s,
                                   const std::locale& loc = std::locale());

  /*! \brief Convert a UTF-32 to a narrow string.
   *
   * Convert a UTF-32 string to a narrow string. This method will encode
   * the characters in the given locale, if possible.
   *
   * In general this will lead to a loss of information. If you wish to
   * preserve all information, you should use toUTF8() instead.
   *
   * \sa toUTF32(const std::string&, const std::locale&), toUTF8(const std::u32string& s)
   *
   * \relates WString
   */
  extern WT_API std::string narrow(const std::u32string& s,
                                   const std::locale& loc = std::locale());
#endif // WT_TARGET_JAVA

  /*! \brief Decode a UTF-8 string a wide string.
   *
   * Decode a UTF-8 string to a wide string. In a UTF-8 encoded unicode string,
   * some unicode characters are represented in more than one byte.
   * This method will decode to extract the proper unicode characters from
   * the string. The resulting string may thus be shorter (has fewer characters)
   * than the original, but does not lead to a loss of information.
   *
   * \sa toUTF8(const std::string& s, const std::locale&), narrow(const std::wstring&, const std::locale&)
   *
   * \relates WString
   */
  extern WT_API std::wstring fromUTF8(const std::string& s);

#ifndef WT_TARGET_JAVA
  /*! \brief Decode a UTF-8 string a UTF-16 string.
   *
   * Decode a UTF-8 string to a UTF-16 string. In a UTF-8 encoded unicode string,
   * some unicode characters are represented in more than one byte.
   * This method will decode to extract the proper unicode characters from
   * the string. The resulting string may thus be shorter (has fewer characters)
   * than the original, but does not lead to a loss of information.
   *
   * \sa toUTF8(const std::string& s, const std::locale&), narrow(const std::u16string&, const std::locale&)
   *
   * \relates WString
   */
  extern WT_API std::u16string utf8ToUTF16(const std::string &s);

  /*! \brief Decode a UTF-8 string a UTF-32 string.
   *
   * Decode a UTF-8 string to a UTF-32 string. In a UTF-8 encoded unicode string,
   * some unicode characters are represented in more than one byte.
   * This method will decode to extract the proper unicode characters from
   * the string. The resulting string may thus be shorter (has fewer characters)
   * than the original, but does not lead to a loss of information.
   *
   * \sa toUTF8(const std::string& s, const std::locale&), narrow(const std::u32string&, const std::locale&)
   *
   * \relates WString
   */
  extern WT_API std::u32string utf8ToUTF32(const std::string &s);
#endif // WT_TARGET_JAVA

  /*! \brief Decode a UTF-8 string into a (narrow) string.
   *
   * Decode a UTF-8 string to a normal string.
   * Not all Unicode characters can be represented in a narrow string,
   * and quite a lot characters will have no equivalent in the target
   * character set, so you may loose information.
   *
   * To distinguish from the other fromUTF8() function, that returns a
   * wstring, the locale is not an optional argument, as in most other
   * locale-conversing functions. You may choose to use the
   * default-constructed std::locale().
   *
   * \sa toUTF8(const std::string& s, const std::locale &),
   *     fromUTF8(const std::string& s)
   *
   * \relates WString
   */
  extern WT_API std::string fromUTF8(const std::string& s,
      const std::locale &loc);

  /*! \brief Encode a wide string to UTF-8.
   *
   * Convert a wide string to UTF-8. This method will encode the given
   * wide string in UTF-8. This may result in a string that is possibly
   * longer (has more characters), but does not lead to a loss of
   * information.
   *
   * \sa fromUTF8(const std::string& s), narrow(const std::wstring&, const std::locale&) 
   *
   * \relates WString
   */
  extern WT_API std::string toUTF8(const std::wstring& s);

#ifndef WT_TARGET_JAVA
  /*! \brief Encode a UTF-16 string to UTF-8.
   *
   * Convert a UTF-16 string to UTF-8. This method will encode the given
   * UTF-16 string in UTF-8. This may result in a string that is possibly
   * longer (has more characters), but does not lead to a loss of
   * information.
   *
   * \sa utf8ToUTF16(const std::string& s), narrow(const std::u16string&, const std::locale&)
   *
   * \relates WString
   */
  extern WT_API std::string toUTF8(const std::u16string& s);

  /*! \brief Encode a UTF-32 string to UTF-8.
   *
   * Convert a UTF-32 string to UTF-8. This method will encode the given
   * UTF-32 string in UTF-8. This may result in a string that is possibly
   * longer (has more characters), but does not lead to a loss of
   * information.
   *
   * \sa utf8ToUTF32(const std::string& s), narrow(const std::u32string&, const std::locale&) 
   *
   * \relates WString
   */
  extern WT_API std::string toUTF8(const std::u32string& s);
#endif // WT_TARGET_JAVA

  /*! \brief Encode a character string (encoding known) to UTF-8.
   *
   * Convert a char * string to UTF-8. This method will encode the given
   * string in UTF-8, assuming that the original string was encoded in the
   * given locale. This conversion does not lead to a loss of information.
   *
   * The reverse operation is in principle narrow(fromUTF8(str), locale).
   *
   * Do not call this function multiple times: toUTF8(toUTF8(str)) is
   * meaningless.
   *
   * \sa toUTF8(const std::wstring& s), fromUTF8(const std::string &),
   * narrow(const std::wstring&, const std::locale&) 
   *
   * \relates WString
   */
  extern WT_API std::string toUTF8(const std::string& s,
      const std::locale &loc = std::locale());

#ifndef WT_TARGET_JAVA
  /*! \brief Convert a wide string to UTF-16.
   *
   * Convert a wide string to UTF-16. If sizeof(wchar_t) == 2, then the
   * resulting string will be a copy of the given string. If sizeof(wchar_t) == 4,
   * the resulting string will be converted from UTF-32 to UTF-16.
   *
   * \relates WString
   */
  extern WT_API std::u16string toUTF16(const std::wstring &s);

  /*! \brief Convert a UTF-32 string to UTF-16
   */
  extern WT_API std::u16string toUTF16(const std::u32string &s);

  /*! \brief Convert a wide string to UTF-32.
   *
   * Convert a wide string to UTF-32. If sizeof(wchar_t) == 4, then the
   * resulting string will be a copy of the given string. If sizeof(wchar_t) == 2,
   * the resulting string will be converted from UTF-16 to UTF-32.
   *
   * \relates WString
   */
  extern WT_API std::u32string toUTF32(const std::wstring &s);

  /*! \brief Convert a UTF-16 string to UTF-32
   */
  extern WT_API std::u32string toUTF32(const std::u16string &s);
#endif // WT_TARGET_JAVA

  extern WT_API std::wostream& streamUTF8(std::wostream &os, const std::string &s);

  // Following is WT_API for testing
  std::string WT_API UTF8Substr(const std::string &s, int begin, int length);
}

#endif // WSTRING_UTIL_H_
