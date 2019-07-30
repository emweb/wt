// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WLOCALIZED_STRINGS_H_
#define WLOCALIZED_STRINGS_H_

#include <string>
#include <Wt/WLocale.h>

namespace Wt {

class WString;

/*! \class LocalizedString Wt/LocalizedString Wt/LocalizedString
 *  \brief The result of resolving a localized string.
 *
 * This struct contains the result (in UTF-8 encoding) of resolving a localized string, consisting of
 * its value, its format (TextFormat::Plain or TextFormat::XHTML), and a success
 * value indicating whether the string was successfully resolved.
 */
struct LocalizedString
{
  /*! \brief Constructor for an unsuccessful localized string result.
   *
   * This constructor sets success to false.
   */
  LocalizedString() : format(TextFormat::Plain), success(false) {}

  /*! \brief Constructor for a successful localized string result.
   *
   * Sets the value to the given string, and the format to the given format,
   * and sets success to true.
   */
  LocalizedString(std::string v, TextFormat f) : value(v), format(f), success(true) {}

  /*! \brief The value of the resolved localized string.
   *
   * This value is UTF-8 encoded
   */
  std::string value;

  /*! \brief The format that the resolved localized string is stored in (TextFormat::Plain or TextFormat::XHTML)
   */
  TextFormat format;

  /*! \brief Indicates whether resolving the string was successful.
   */
  bool success;

#ifndef WT_TARGET_JAVA
  /*! \brief Bool conversion, for checking success.
   */
  explicit inline operator bool() const { return success; }
  inline bool operator!() const { return !success; }
#endif // WT_TARGET_JAVA
};

/*! \class WLocalizedStrings Wt/WLocalizedStrings.h Wt/WLocalizedStrings.h
 *  \brief An abstract class that provides support for localized strings.
 *
 * This abstract class provides the content to localized WStrings, by
 * resolving the key to a string using the current application locale.
 *
 * \sa WString::tr(), WApplication::setLocalizedStrings()
 */
class WT_API WLocalizedStrings
{
public:
  /*! \brief Destructor.
   */
  virtual ~WLocalizedStrings();

  /*! \brief Purges memory resources, if possible.
   * 
   * This is called afer event handling, and is an opportunity to
   * conserve memory inbetween events, by freeing memory used for
   * cached key/value bindings, if applicable.
   *
   * The default implementation does nothing.
   */
  virtual void hibernate();

  /*! \brief Resolves a key in the given locale.
   * 
   * This method is used by WString to obtain the UTF-8 value corresponding
   * to a key in the given locale.
   *
   * Returns a successful LocalizedString if the key could be resolved.
   *
   * \sa WString::tr()
   */
  virtual LocalizedString resolveKey(const WLocale& locale, const std::string& key) = 0;

  /*! \brief Resolves the plural form of a key in the given locale.
   * 
   * This method is used by WString to obtain the UTF-8 value
   * corresponding to a key in the current locale, taking into account
   * the possibility of multiple plural forms, and chosing the right
   * plural form based on the \p amount passed.
   *
   * Throws a std::logic_error if the underlying implementation does not
   * provide support for plural internationalized strings.
   *
   * Returns a successful LocalizedString if the key could be resolved.
   *
   * \sa WString::trn()
   */
#ifndef WT_TARGET_JAVA
  virtual LocalizedString resolvePluralKey(const WLocale& locale,
				const std::string& key, 
                                ::uint64_t amount);

  /*! \brief Utility method to evaluate a plural expression.
   *
   * This evaluates C expressions such as used by ngettext for a particular
   * value, which can be useful to implement plural key resolution.
   *
   * \sa resolvePluralKey() 
   */
  static int evaluatePluralExpression(const std::string &expression,
				      ::uint64_t n);

#endif // WT_TARGET_JAVA

};

}

#endif // WLOCALIZED_STRINGS_H_
