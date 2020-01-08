// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WSTRING_H_
#define WSTRING_H_

#include <Wt/WDllDefs.h>
#include <Wt/WGlobal.h>

#include <string>
#include <vector>
#include <iosfwd>
#include <locale>

namespace Wt {

/*! \class WString Wt/WString.h Wt/WString.h
 *  \brief A value class which describes a locale-aware unicode string.
 *
 * %Wt offers this string to facilitate handling of unicode text
 * through the user interface, and to offer support for localized text
 * using message resource bundles.
 *
 * A %WString may be constructed from a std::string, std::wstring or
 * c-style strings (const char * and const wchar_t *), and converted
 * to each of these strings taking into account the locale in which
 * the %Wt application runs on the web server. Independent of the
 * locale on the web server, you may convert from and to UTF8 unicode
 * encoded std::strings.
 *
 * By using the static functions WString::tr() (or WWidget::tr()), one
 * may construct a localized string. The key is used to retrieve its
 * current value from the application's message-resource
 * bundles.
 *
 * Argument place holder in a string, denoted using {<i>n</i>} for the
 * <i>n</i>'th argument, may be substituted by values set using
 * arg(int) and arg(std::string).
 *
 * %WString is used by all built-in widgets for displayed text. By
 * calling WApplication::setLocale() or WApplication::refresh(), the
 * contents of every %WString is reevaluated in the new locale, by
 * calling refresh(). In this way, the contents of the whole user
 * interface is adapted to a possibly changed locale.
 *
 * To benefit from the localization properties of %WString, you should
 * design your own widget classes to use %WString in their interface
 * for any text that is displayed. In this way, your own widgets will
 * automatically, and without any extra effort, participate in a
 * relocalization triggered by WApplication::setLocale().
 *
 * This string class does not provide anything more than basic
 * manipulations. Instead, you should convert to a standard library
 * string class to manipulate the string contents and perform string
 * algorithms on them. In particular, we recommend to use the
 * conversion methods toUTF8(), fromUTF8() to convert from and to UTF8
 * encoded std::strings. In this way, you can support the whole
 * unicode character set, with backward compatible support for the
 * standard 7-bit ASCII set. Since %WString internally uses
 * UTF8-encoding, and UTF8-encoding is used by the library for
 * communication with the browser, there is no actual conversion
 * overhead. Only when you need to run string algorithms that require
 * the actual length of the string in number of characters, you would
 * need to convert to a wide string representation such as
 * std::wstring.
 *
 * \sa WApplication::messageResourceBundle()
 * \sa WApplication::locale()
 */
class WT_API WString
{
public:
  /*! \brief Sets the encoding for
   *         \link Wt::CharEncoding::Default CharEncoding::Default\endlink
   *
   * WString::setDefaultEncoding() controls the behaviour of \link
   * Wt::CharEncoding::Default CharEncoding::Default\endlink. If not modified, the
   * default encoding will be the CharEncoding::UTF8.
   *
   * Since this is a system-wide setting, and not a per-session setting,
   * you should call this function before any session is created, e.g. in
   * main() before calling WRun().
  */
  static void setDefaultEncoding(Wt::CharEncoding encoding);

  /*! \brief Creates an empty string
   *
   * Create a literal string with empty contents ("").
   */
  WString();

  /*! \brief Creates a %WString from a wide C string.
   *
   * The wide string is implicitly converted to proper unicode. Note
   * that there are known issues with the portability of wchar_t since
   * its width and encoding are platform dependent.
   */
  WString(const wchar_t *value);

  /*! \brief Copy constructor
   */
  WString(const WString& other);

  /*! \brief Move constructor
   */
  WString(WString&& other);

  /*! \brief Creates a %WString from a wide C++ string
   *
   * The wide string is implicitly converted to proper unicode. Note
   * that there are known issues with the portability of wchar_t since
   * its width and encoding are platform dependent.
   */
  WString(const std::wstring& value);

#ifndef WT_TARGET_JAVA
  /*! \brief Creates a %WString from a UTF-16 C string.
   *
   * The UTF-16 string is implicitly converted to UTF-8.
   */
  WString(const char16_t *value);

  /*! \brief Creates a %WString from a UTF-16 C++ string
   *
   * The UTF-16 string is implicitly converted to UTF-8.
   */
  WString(const std::u16string &value);

  /*! \brief Creates a %WString from a UTF-32 C string.
   *
   * The UTF-32 string is implicitly converted to UTF-8.
   */
  WString(const char32_t *value);

  /*! \brief Creates a %WString from a UTF-32 C++ string
   *
   * The UTF-32 string is implicitly converted to UTF-8.
   */
  WString(const std::u32string &value);
#endif // WT_TARGET_JAVA

  /*! \brief Creates a %WString from a C string.
   *
   * The C string is implicitly converted to unicode. When
   * \p encoding is \link Wt::CharEncoding::Local CharEncoding::Local\endlink,
   * the current locale is used to interpret the C string. When
   * encoding is \link Wt::CharEncoding::UTF8 CharEncoding::UTF8\endlink, the C string is
   * interpreted as a CharEncoding::UTF8 encoded unicode string.
   *
   * WString::setDefaultEncoding() controls the behaviour of
   * \link Wt::CharEncoding::Default CharEncoding::Default\endlink. Use it to set a
   * system-wide default format for C style strings (e.g. to UTF-8).
   */
  WString(const char *value,
	  CharEncoding encoding = CharEncoding::Default);

  /*! \brief Creates a %WString from a C string.
   *
   * The C string is implicitly converted to unicode. The
   * string is interpreted within the character set of the given locale.
   */
  WString(const char *value, const std::locale &loc);

  /*! \brief Creates a %WString from a C++ string.
   *
   * The C++ string is implicitly converted to unicode. When
   * \p encoding is \link Wt::CharEncoding::Local CharEncoding::Local\endlink,
   * the current locale is used to interpret the C++ string. When
   * encoding is \link Wt::CharEncoding::UTF8 CharEncoding::UTF8\endlink, the C++ string is
   * interpreted as a CharEncoding::UTF8 encoded unicode string.
   *
   * WString::setDefaultEncoding() controls the behaviour of
   * \link Wt::CharEncoding::Default CharEncoding::Default\endlink. Use it to set a
   * system-wide default format for C style strings (e.g. to UTF-8).

   */
  WString(const std::string& value,
	  CharEncoding encoding = CharEncoding::Default);

  /*! \brief Creates a %WString from a C++ string.
   *
   * The C++ string is implicitly converted to unicode. When
   * \p encoding is \link Wt::CharEncoding::Local CharEncoding::Local\endlink,
   * the current locale is used to interpret the C++ string. When
   * encoding is \link Wt::CharEncoding::UTF8 CharEncoding::UTF8\endlink, the C++ string is
   * interpreted as a CharEncoding::UTF8 encoded unicode string.
   *
   * WString::setDefaultEncoding() controls the behaviour of
   * \link Wt::CharEncoding::Default CharEncoding::Default\endlink. Use it to set a
   * system-wide default format for C style strings (e.g. to UTF-8).

   */
  WString(std::string&& value,
	  CharEncoding encoding = CharEncoding::Default);

  /*! \brief Creates a %WString from a C++ string.
   *
   * The C++ string is implicitly converted to unicode. The
   * string is interpreted within the character set of the given locale.
   */
  WString(const std::string& value, const std::locale &loc);

  /*! \brief Destructor
   */
  ~WString();

  WString trim() const;

  /*! \brief Copy assignment operator
   *
   * Copy another string into this string.
   */
  WString& operator= (const WString& rhs);

  /*! \brief Move assignment operator
   *
   * Move another string into this string.
   */
  WString& operator= (WString&& rhs);

  /*! \brief Comparison operator
   *
   * Compares two strings and returns \c true if the strings are exactly
   * the same. This may require evaluating a localized string in the
   * current locale.
   */
  bool operator== (const WString& rhs) const;

  /*! \brief Comparison operator
   *
   * Compares to strings lexicographically. This may require
   * evaluating a localized string in the current locale. The unicode
   * representation of the strings are compared.
   */
  bool operator< (const WString& rhs) const;

  /*! \brief Comparison operator
   *
   * Compares to strings lexicographically. This may require
   * evaluating a localized string in the current locale. The unicode
   * representation of the strings are compared.
   */
  bool operator> (const WString& rhs) const;

#ifdef WT_TARGET_JAVA
  int compareTo(const WString& rhs) const;
#endif

  /*! \brief Self-concatenation operator
   *
   * Appends a string to the current value. If the string was localized,
   * this automatically converts it to a literal string, by evaluating the
   * string using the current WLocale.
   */
  WString& operator+= (const WString& rhs);

  /*! \brief Self-concatenation operator
   *
   * Appends a string to the current value. If the string was localized,
   * this automatically converts it to a literal string, by evaluating the
   * string using the current WLocale.
   */
  WString& operator+= (const std::wstring& rhs);

#ifndef WT_TARGET_JAVA
  /*! \brief Self-concatenation operator
   *
   * Appends a string to the current value. If the string was localized,
   * this automatically converts it to a literal string, by evaluating the
   * string using the current WLocale.
   */
  WString& operator+= (const std::u16string& rhs);

  /*! \brief Self-concatenation operator
   *
   * Appends a string to the current value. If the string was localized,
   * this automatically converts it to a literal string, by evaluating the
   * string using the current WLocale.
   */
  WString& operator+= (const std::u32string& rhs);
#endif // WT_TARGET_JAVA

  /*! \brief Self-concatenation operator
   *
   * Appends a string to the current value. If the string was localized,
   * this automatically converts it to a literal string, by evaluating the
   * string using the current WLocale.
   */
  WString& operator+= (const wchar_t *rhs);

#ifndef WT_TARGET_JAVA
  /*! \brief Self-concatenation operator
   *
   * Appends a string to the current value. If the string was localized,
   * this automatically converts it to a literal string, by evaluating the
   * string using the current WLocale.
   */
  WString& operator+= (const char16_t *rhs);

  /*! \brief Self-concatenation operator
   *
   * Appends a string to the current value. If the string was localized,
   * this automatically converts it to a literal string, by evaluating the
   * string using the current WLocale.
   */
  WString& operator+= (const char32_t *rhs);
#endif // WT_TARGET_JAVA

  /*! \brief Self-concatenation operator
   *
   * Appends a string to the current value. The right hand side is
   * interpreted in the server locale and converted to unicode. If the
   * string was localized, this automatically converts it to a literal
   * string, by evaluating the string using the current WLocale.
   */
  WString& operator+= (const std::string& rhs);

  /*! \brief Self-concatenation operator
   *
   * Appends a string to the current value. The right hand side is
   * interpreted in the server locale and converted to unicode. If the
   * string was localized, this automatically converts it to a literal
   * string, by evaluating the string using the current WLocale.
   */
  WString& operator+= (const char *rhs);

  /*! \brief Returns whether the string is empty.
   */
  bool empty() const;

  /*! \brief Creates a %WString from a UTF-8 encoded string.
   *
   * This is equivalent to using the constructor WString(\p value,
   * CharEncoding::UTF8).
   *
   * When \p checkValid is \c true, the UTF-8 encoding is validated. You
   * should enable this only if you cannot trust the origin of the string.
   * The library uses this internally whenever it receives data from the
   * browser (in UTF-8 format).
   */
  static WString fromUTF8(const std::string& value, bool checkValid = false);

  /*! \brief Creates a %WString from a UTF-8 encoded string.
   *
   * This is equivalent to using the constructor WString(\p value,
   * CharEncoding::UTF8).
   *
   * When \p checkValid is \c true, the UTF-8 encoding is validated. You
   * should enable this only if you cannot trust the origin of the string.
   * The library uses this internally whenever it receives data from the
   * browser (in UTF-8 format).
   */
  static WString fromUTF8(std::string&& value, bool checkValid = false);

  /*! \brief Creates a %WString from a UTF-8 unicode encoded string.
   *
   * This is equivalent to using the constructor WString(\p value,
   * CharEncoding::UTF8).
   *
   * When \p checkValid is \c true, the UTF-8 encoding is validated. You
   * should enable this only if you cannot trust the origin of the string.
   * The library uses this internally whenever it receives data from the
   * browser (in UTF-8 format).
   */
  static WString fromUTF8(const char *value, bool checkValid = false);

  /*! \brief Returns the value as a UTF-8 encoded string.
   *
   * For a localized string, returns the current localized value. If
   * the localized string is formatted as XML, this will unescape all
   * XML escapes. Literal strings will remain unchanged.
   *
   * \sa fromUTF8(), toXhtmlUTF8()
   */
  std::string toUTF8() const;

  /*! \brief Returns the value as a UTF-8 encoded XHTML string.
   *
   * For a localized string, returns the current localized value. If
   * the localized string is formatted as plaintext, the localized string
   * will be escaped. Literal strings will remain unchanged.
   *
   * \sa toUTF8()
   */
  std::string toXhtmlUTF8() const;

  /*! \brief Creates a localized string from a key.
   *
   * Whenever the value of the string is needed, the key is used for a
   * lookup in the application message resource bundles taking into
   * account the current application locale. If the key cannot be
   * resolved, its value is set to '??key??'.
   *
   * \sa WApplication::locale(), WApplication::localizedStrings()
   */
  static WString tr(const char *key);

  /*! \brief Creates a localized string with the specified key.
   *
   * \sa tr(const char *)
   */
  static WString tr(const std::string& key);

  /*! \brief Creates a localized string from a key for a number \p n.
   *
   * Whenever the value of the string is needed, the \p key is used
   * for a lookup in the application message resource bundles taking
   * into account the current application locale. This function
   * fetches the appropriate plural case for the translation
   * corresponding to the quantity \p n. Note that usually, your
   * string will have a place-holder for the value of \p n, and thus
   * you will also need to bind \p as an argument.
   *
   * For example, consider a string "quantity.cars" with two plural cases:
   * - n == 1: "{1} car"
   * - n != 1: "{1} cars"
   *
   * You would use the following to use the string:
   * \if cpp
   * \code
   * Wt::WString::trn("quantity.cars", cars).arg(cars);
   * \endcode
   * \else
   * \code
   * WString.trn("quantity.cars", cars).arg(cars);
   * \endcode
   * \endif
   *
   * If the \p key cannot be resolved, its value is set to '??key??'.
   *
   * \sa tr()
   */
  static WString trn(const char *key, ::uint64_t n);

  /*! \brief Creates a localized string with the specified key for a
      number \c n.
   *
   * \sa trn(const char *)
   */
  static WString trn(const std::string& key, ::uint64_t n);

  /*! \brief Returns the value as a wide C++ string.
   *
   * A localized string is resolved using the WApplication::localizedStrings().
   *
   * Argument place holders are substitued with actual arguments.
   */
  std::wstring value() const;

#ifndef WT_TARGET_JAVA
  /*! \brief Returns the value as a UTF-16 C++ string.
   *
   * A localized string is resolved using the WApplication::localizedStrings().
   *
   * Argument place holders are substitued with actual arguments.
   */
  std::u16string toUTF16() const;

  /*! \brief Returns the value as a UTF-32 C++ string.
   *
   * A localized string is resolved using the WApplication::localizedStrings().
   *
   * Argument place holders are substitued with actual arguments.
   */
  std::u32string toUTF32() const;
#endif // WT_TARGET_JAVA

  /*! \brief Returns the value as a narrow C++ string.
   *
   * A localized string is resolved using the WApplication::localizedStrings().
   *
   * Argument place holders are substitued with actual arguments.
   *
   * Any wide character is narrowed using the provided locale, possibly
   * losing information. If you wish to keep all information, use toUTF8()
   * instead, which encodes wide characters in the string.
   *
   * \sa toUTF8()
   */
  std::string narrow(const std::locale &loc = std::locale()) const;

  /*! \brief Returns the value as a wide C++ string.
   *
   * A localized string is resolved using the WApplication::localizedStrings().
   *
   * Argument place holders are substitued with actual arguments.
   */
  operator std::wstring() const;

#ifndef WT_TARGET_JAVA
  /*! \brief Returns the value as a UTF-16 C++ string.
   *
   * A localized string is resolved using the WApplication::localizedStrings().
   *
   * Argument place holders are substitued with actual arguments.
   */
  operator std::u16string() const;

  /*! \brief Returns the value as a UTF-32 C++ string.
   *
   * A localized string is resolved using the WApplication::localizedStrings().
   *
   * Argument place holders are substitued with actual arguments.
   */
  operator std::u32string() const;
#endif // WT_TARGET_JAVA

  /*! \brief Returns whether the string is literal or localized.
   *
   * \sa tr()
   */
  bool literal() const { return !impl_ || impl_->key_.empty(); }

  /*! \brief Returns the key for a localized string.
   *
   * When the string is literal, the result is undefined.
   */
  const std::string key() const;

  /*! \brief Substitutes the next positional argument with a string value.
   *
   * In the string, the \p n-th argument is referred to as using
   * {\p n}.
   *
   * For example: the string "<tt>{1} bought {2} apples in the
   * shop.</tt>" with first argument value "<tt>Bart</tt>" and second
   * argument value <tt>5</tt> becomes: "<tt>Bart bought 5 apples in
   * the shop.</tt>"
   */
  WString& arg(const std::wstring& value);

  /*! \brief Substitutes the next positional argument with a string value.
   *
   * In the string, the \p n-th argument is referred to as using
   * {\p n}.
   *
   * For example: the string "<tt>{1} bought {2} apples in the
   * shop.</tt>" with first argument value "<tt>Bart</tt>" and second
   * argument value <tt>5</tt> becomes: "<tt>Bart bought 5 apples in
   * the shop.</tt>"
   */
  WString& arg(const wchar_t *value);

#ifndef WT_TARGET_JAVA
  /*! \brief Substitutes the next positional argument with a string value.
   *
   * In the string, the \p n-th argument is referred to as using
   * {\p n}.
   *
   * For example: the string "<tt>{1} bought {2} apples in the
   * shop.</tt>" with first argument value "<tt>Bart</tt>" and second
   * argument value <tt>5</tt> becomes: "<tt>Bart bought 5 apples in
   * the shop.</tt>"
   */
  WString& arg(const std::u16string& value);

  /*! \brief Substitutes the next positional argument with a string value.
   *
   * In the string, the \p n-th argument is referred to as using
   * {\p n}.
   *
   * For example: the string "<tt>{1} bought {2} apples in the
   * shop.</tt>" with first argument value "<tt>Bart</tt>" and second
   * argument value <tt>5</tt> becomes: "<tt>Bart bought 5 apples in
   * the shop.</tt>"
   */
  WString& arg(const char16_t *value);

  /*! \brief Substitutes the next positional argument with a string value.
   *
   * In the string, the \p n-th argument is referred to as using
   * {\p n}.
   *
   * For example: the string "<tt>{1} bought {2} apples in the
   * shop.</tt>" with first argument value "<tt>Bart</tt>" and second
   * argument value <tt>5</tt> becomes: "<tt>Bart bought 5 apples in
   * the shop.</tt>"
   */
  WString& arg(const std::u32string& value);

  /*! \brief Substitutes the next positional argument with a string value.
   *
   * In the string, the \p n-th argument is referred to as using
   * {\p n}.
   *
   * For example: the string "<tt>{1} bought {2} apples in the
   * shop.</tt>" with first argument value "<tt>Bart</tt>" and second
   * argument value <tt>5</tt> becomes: "<tt>Bart bought 5 apples in
   * the shop.</tt>"
   */
  WString& arg(const char32_t *value);
#endif // WT_TARGET_JAVA

  /*! \brief Substitutes the next positional argument with a string value.
   *
   * In the string, the \p n-th argument is referred to as using
   * {\p n}.
   *
   * For example: the string "<tt>{1} bought {2} apples in the
   * shop.</tt>" with first argument value "<tt>Bart</tt>" and second
   * argument value <tt>5</tt> becomes: "<tt>Bart bought 5 apples in
   * the shop.</tt>"
   */
  WString& arg(const std::string& value,
	       CharEncoding encoding = CharEncoding::Default);

  /*! \brief Substitutes the next positional argument with a string value.
   *
   * In the string, the \p n-th argument is referred to as using
   * {\p n}.
   *
   * For example: the string "<tt>{1} bought {2} apples in the
   * shop.</tt>" with first argument value "<tt>Bart</tt>" and second
   * argument value <tt>5</tt> becomes: "<tt>Bart bought 5 apples in
   * the shop.</tt>"
   */
  WString& arg(const char *value,
	       CharEncoding encoding = CharEncoding::Default);

  /*! \brief Substitutes the next positional argument with a string value.
   *
   * In the string, the \p n-th argument is referred to as using
   * {\p n}.
   *
   * For example: the string "<tt>{1} bought {2} apples in the
   * shop.</tt>" with first argument value "<tt>Bart</tt>" and second
   * argument value <tt>5</tt> becomes: "<tt>Bart bought 5 apples in
   * the shop.</tt>"
   */
  WString& arg(const WString& value);

  /*! \brief Substitutes the next positional argument with an integer value.
   *
   * In the string, the \p n-th argument is reffered to as using
   * {\p n}.
   *
   * For example: the string "<tt>{1} bought {2} apples in the
   * shop.</tt>" with first argument value "<tt>Bart</tt>" and second
   * argument value <tt>5</tt> becomes: "<tt>Bart bought 5 apples in
   * the shop.</tt>"
   */
  WString& arg(int value);

  /*! \brief Substitutes the next positional argument with an unsigned value.
   *
   * \sa arg()
   */
  WString& arg(unsigned value);

  /*! \brief Substitutes the next positional argument with an integer value.
   *
   * In the string, the \p n-th argument is reffered to as using
   * {\p n}.
   *
   * For example: the string "<tt>{1} bought {2} apples in the
   * shop.</tt>" with first argument value "<tt>Bart</tt>" and second
   * argument value <tt>5</tt> becomes: "<tt>Bart bought 5 apples in
   * the shop.</tt>"
   */
  WString& arg(long value);

  /*! \brief Substitutes the next positional argument with an unsigned value.
   *
   * \sa arg()
   */
  WString& arg(unsigned long value);

  /*! \brief Substitutes the next positional argument with an integer value.
   *
   * In the string, the \p n-th argument is reffered to as using
   * {\p n}.
   *
   * For example: the string "<tt>{1} bought {2} apples in the
   * shop.</tt>" with first argument value "<tt>Bart</tt>" and second
   * argument value <tt>5</tt> becomes: "<tt>Bart bought 5 apples in
   * the shop.</tt>"
   */
  WString& arg(long long value);

  /*! \brief Substitutes the next positional argument with an unsigned value.
   *
   * \sa arg()
   */
  WString& arg(unsigned long long value);

  /*! \brief Substitutes the next positional argument with a double value.
   *
   * In the string, the \p n-th argument is reffered to as using
   * {\p n}.
   *
   * For example: the string "<tt>{1} bought {2} apples in the
   * shop.</tt>" with first argument value "<tt>Bart</tt>" and second
   * argument value <tt>5</tt> becomes: "<tt>Bart bought 5 apples in
   * the shop.</tt>"
   */
  WString& arg(double value);

  /*! \brief Returns the list of arguments
   */
  const std::vector<WString>& args() const;

  /*! \brief Refreshes the string.
   *
   * For a localized string, its value is resolved again.
   *
   * Returns whether the value has (potentially) changed.
   */
  bool refresh();

  /*! \brief Returns the string as a JavaScript literal
   *
   * The \p delimiter may be a single or double quote.
   *
   * \sa WWebWidget::jsStringLiteral()
   */
  std::string jsStringLiteral(char delimiter = '\'') const;

#ifdef WT_CNOR
  WString& operator+(const char *);
  WString& operator+(const WString &);
#endif

  /*! \brief Comparison operator
   *
   * Compares two strings and returns \c true if the strings are not exactly
   * the same. This may require evaluating a localized string in the
   * current locale.
   */
  bool operator!= (const WString& rhs) const { return !(*this == rhs); }

  /*! \brief An empty string.
   */
  static const WString Empty;

  static void checkUTF8Encoding(std::string& value);

private:
  WString(const char *key, bool, ::uint64_t n = -1);

  std::string utf8_;

  std::string resolveKey(TextFormat format) const;

  void makeLiteral();

  struct Impl {
    std::string key_;
    std::vector<WString> arguments_;
    ::int64_t n_;

    Impl();
  };

  static std::vector<WString> stArguments_;

  void createImpl();

  Impl *impl_;
  static CharEncoding defaultEncoding_;
  static CharEncoding realEncoding(CharEncoding encoding);
};

#ifndef WT_CNOR

/*! \brief Short hand for WString(const char * value, CharEncoding::UTF8)
 *
 * \relates WString
 */
extern WT_API WString utf8(const char *value);

/*! \brief Short hand for WString(const std::string& value, CharEncoding::UTF8)
 *
 * \relates WString
 */
extern WT_API WString utf8(const std::string& value);

/*! \brief Concatenate two WStrings
 *
 * \relates WString
 */
extern WT_API WString operator+ (const WString& lhs, const WString& rhs);

/*! \brief Conatenate a WString with a C++ wide string
 *
 * \relates WString
 */
extern WT_API WString operator+ (const WString& lhs, const std::wstring& rhs);

/*! \brief Conatenate a WString with a C++ UTF-16 string
 *
 * \relates WString
 */
extern WT_API WString operator+ (const WString& lhs, const std::u16string& rhs);

/*! \brief Conatenate a WString with a C++ UTF-32 string
 *
 * \relates WString
 */
extern WT_API WString operator+ (const WString& lhs, const std::u32string& rhs);

/*! \brief Conatenate a WString with a C wide string
 *
 * \relates WString
 */
extern WT_API WString operator+ (const WString& lhs, const wchar_t *rhs);

/*! \brief Conatenate a WString with a C UTF-16 string
 *
 * \relates WString
 */
extern WT_API WString operator+ (const WString& lhs, const char16_t *rhs);

/*! \brief Conatenate a WString with a C UTF-32 string
 *
 * \relates WString
 */
extern WT_API WString operator+ (const WString& lhs, const char32_t *rhs);

/*! \brief Conatenate a WString with a C++ string
 *
 * \relates WString
 */
extern WT_API WString operator+ (const WString& lhs, const std::string& rhs);

/*! \brief Conatenate a WString with a C string
 *
 * \relates WString
 */
extern WT_API WString operator+ (const WString& lhs, const char *rhs);

/*! \brief Conatenate a C++ wide string with a WString
 *
 * \relates WString
 */
extern WT_API WString operator+ (const std::wstring& lhs, const WString& rhs);

/*! \brief Conatenate a C++ UTF-16 string with a WString
 *
 * \relates WString
 */
extern WT_API WString operator+ (const std::u16string& lhs, const WString& rhs);

/*! \brief Conatenate a C++ UTF-32 string with a WString
 *
 * \relates WString
 */
extern WT_API WString operator+ (const std::u32string& lhs, const WString& rhs);

/*! \brief Conatenate a C wide string with a WString
 *
 * \relates WString
 */
extern WT_API WString operator+ (const wchar_t *lhs, const WString& rhs);

/*! \brief Conatenate a C UTF-16 string with a WString
 *
 * \relates WString
 */
extern WT_API WString operator+ (const char16_t *lhs, const WString& rhs);

/*! \brief Conatenate a C UTF-32 string with a WString
 *
 * \relates WString
 */
extern WT_API WString operator+ (const char32_t *lhs, const WString& rhs);

/*! \brief Conatenate a C++ string with a WString
 *
 * \relates WString
 */
extern WT_API WString operator+ (const std::string& lhs, const WString& rhs);

/*! \brief Conatenate a C string with a WString
 *
 * \relates WString
 */
extern WT_API WString operator+ (const char *lhs, const WString& rhs);

/*! \brief Compare a C string with a WString
 *
 * \relates WString
 */
extern WT_API bool operator== (const char *lhs, const WString& rhs);

/*! \brief Compare a C wide string with a WString
 *
 * \relates WString
 */
extern WT_API bool operator== (const wchar_t *lhs, const WString& rhs);

/*! \brief Compare a C UTF-16 string with a WString
 *
 * \relates WString
 */
extern WT_API bool operator== (const char16_t *lhs, const WString& rhs);

/*! \brief Compare a C UTF-32 string with a WString
 *
 * \relates WString
 */
extern WT_API bool operator== (const char32_t *lhs, const WString& rhs);

/*! \brief Compare a C++ string with a WString
 *
 * \relates WString
 */
extern WT_API bool operator== (const std::string& lhs, const WString& rhs);

/*! \brief Compare a C++ wide string with a WString
 *
 * \relates WString
 */
extern WT_API bool operator== (const std::wstring& lhs, const WString& rhs);

/*! \brief Compare a C++ UTF-16 string with a WString
 *
 * \relates WString
 */
extern WT_API bool operator== (const std::u16string& lhs, const WString& rhs);

/*! \brief Compare a C++ UTF-32 string with a WString
 *
 * \relates WString
 */
extern WT_API bool operator== (const std::u32string& lhs, const WString& rhs);

/*! \brief Compare a C string with a WString
 *
 * \relates WString
 */
extern WT_API bool operator!= (const char *lhs, const WString& rhs);

/*! \brief Compare a C wide string with a WString
 *
 * \relates WString
 */
extern WT_API bool operator!= (const wchar_t *lhs, const WString& rhs);

/*! \brief Compare a C UTF-16 string with a WString
 *
 * \relates WString
 */
extern WT_API bool operator!= (const char16_t *lhs, const WString& rhs);

/*! \brief Compare a C UTF-32 string with a WString
 *
 * \relates WString
 */
extern WT_API bool operator!= (const char32_t *lhs, const WString& rhs);

/*! \brief Compare a C++ string with a WString
 *
 * \relates WString
 */
extern WT_API bool operator!= (const std::string& lhs, const WString& rhs);

/*! \brief Compare a C++ wide string with a WString
 *
 * \relates WString
 */
extern WT_API bool operator!= (const std::wstring& lhs, const WString& rhs);

/*! \brief Compare a C++ UTF-16 string with a WString
 *
 * \relates WString
 */
extern WT_API bool operator!= (const std::u16string& lhs, const WString& rhs);

/*! \brief Compare a C++ UTF-32 string with a WString
 *
 * \relates WString
 */
extern WT_API bool operator!= (const std::u32string& lhs, const WString& rhs);

/*! \brief Output a WString to a C++ wide stream
 *
 * \relates WString
 */
extern WT_API std::wostream& operator<< (std::wostream& lhs, const WString& rhs);

/*! \brief Output a WString to a C++ stream
 *
 * The string is narrowed using the currently global C++ locale, possibly
 * losing information.
 * \relates WString
 */
extern WT_API std::ostream& operator<< (std::ostream& lhs, const WString& rhs);

#endif // WT_CNOR

#ifdef WT_TARGET_JAVA
/* To emit javadoc links */
const WString WString::Empty;
WString WString::tr(const char *key) { }
#endif

}

#endif // WSTRING_H_
