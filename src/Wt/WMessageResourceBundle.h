// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WMESSAGE_RESOURCE_BUNDLE_
#define WMESSAGE_RESOURCE_BUNDLE_

#include <vector>
#include <set>
#include <Wt/WFlags.h>
#include <Wt/WLocalizedStrings.h>

namespace Wt {

class WMessageResources;

/*! \class WMessageResourceBundle Wt/WMessageResourceBundle.h Wt/WMessageResourceBundle.h
 *  \brief Support for localized strings using XML files.
 *
 * The resource bundle manages a number of resource files, which allow
 * the developer to conceptually manage its messages in a number of
 * libraries.
 *
 * For example, a WApplication may have a generic message library, that is
 * shared with many other libraries, with re-occurring messages (such as
 * 'welcome', 'add to shopping cart', and 'pay'), and a specific message
 * library for specific messages.
 *
 * Usage example:
 *
 * XML file "general.xml":
 * \verbatim
 <?xml version="1.0" encoding="UTF-8"?>
 <messages>

   <message id='welcome-text'>
     Welcome dear visiter, {1} of the WFooBar magic website !
   </message>

   <message id='company-policy'>
     The company policy is to <b>please our share-holders</b>.
   </message>

 </messages>
\endverbatim
 *
 * The encodings supported are ASCII, UTF-8 (recommended) or UTF-16.
 *
 * \if cpp
 * Use this resource bundle in your program:
 * \code
 * // load the message resource bundle
 * Wt::WApplication *app = Wt::WApplication::instance();
 * app->messageResourceBundle().use("general");
 *
 * // resolve a string using the resource bundle
 * auto welcome = std::make_unique<Wt::WText>(
 *                  tr("welcome-text").arg("Bart"));
 * \endcode
 * \endif
 *
 * To refer the two messages defined in this resource file, use
 * WString::tr("welcome-text").\link WString::arg()
 * arg\endlink(userName) or WWidget::tr("company-policy").
 *
 * <h3>Plural forms</h3>
 *
 * The XML format supports plural noun forms, and allows the
 * definition of different cases of plural nouns, per language. The
 * amount of cases per noun may differ between language families, as
 * does the expression to transform a number associated with a noun
 * into a case id. Two attributes for the <tt>messages</tt> XML
 * element configure the plural support:
 *
 * - the 'nplurals' attribute indicates the number of plural cases;
 * cases are numbered 0 .. (<i>nplurals</i> - 1).
 * - the 'plural' attribute is an expression which evaluates converts
 * a count <i>n</i> into a plural case. This expression should use the
 * C language syntax, with the exception that no negative numbers are
 * allowed, numbers must be integers, and the only allowed variable is
 * n, which corresponds to the amount passed to WString::trn(). The
 * resulting value must be a plural case: 0 .. (<i>nplurals</i> - 1).
 *
 * Using WString::trn(), you can pass the count <i>n</i> that is used
 * to select a suitable plural case.
 *
 * As an example, consider an XML resource file describing Polish translations,
 * which enlists all plural cases of the noun 'plik' (<i>file</i> in Polish).
 * The noun is used this way: 
 * - 1 plik
 * - 2,3,4 pliki
 * - 5-21 pliko'w
 * - 22-24 pliki
 * - 25-31 pliko'w
 *
 * \verbatim
<?xml version="1.0" encoding="UTF-8"?>
  <messages nplurals="3" 
            plural="n==1 ? 0 : n%10>=2 && n%10<=4 && (n%100<10 || n%100>=20) ? 1 : 2">

    <message id="file">
      <plural case="0">{1} plik</plural>
      <plural case="1">{1} pliki</plural>
      <plural case="2">{1} pliko'w</plural>
    </message>

  </messages>
\endverbatim
 *
 * To message defined in this resource file can then be used using
 * WString::trn("file", n).
 *
 * \sa WApplication::locale(), WString::tr(), WString::trn()
 */
class WT_API WMessageResourceBundle : public WLocalizedStrings
{
public:
  /*! \brief Creates a message resource bundle.
   */
  WMessageResourceBundle();

  WMessageResourceBundle &operator=(const WMessageResourceBundle &) = delete;
  WMessageResourceBundle(const WMessageResourceBundle &) = delete;

  virtual ~WMessageResourceBundle();

  /*! \brief Adds a (series) of message resource files to be used.
   *
   * The \p path is not a URI, and relative paths will be resolved
   * with respect to the working directory of the server. The XML
   * files do not need to be deployed in the web server's docroot.
   *
   * When you give as \p path: /path/to/name, then the following message
   * resource files will be used:
   *  - /path/to/name.xml (default, English)
   *  - /path/to/name_nl.xml (for Dutch)
   *  - /path/to/name_fr.xml (for French)
   *  - etc...
   *
   * The message file that is used depends on the application's locale.
   *
   * \sa WApplication::locale()
   */
  void use(const std::string& path, bool loadInMemory = true);

  void useBuiltin(const char *xmlbundle);

  /*! \brief Returns a set of all keys in this bundle.
   *
   * Returns a set of all keys connected with this WMessageResources,
   * within the scope provided as parameter.
   */
  const std::set<std::string> keys(const WLocale& locale) const;

  virtual void hibernate() override;

  virtual LocalizedString resolveKey(const WLocale& locale, const std::string& key) override;
  virtual LocalizedString resolvePluralKey(const WLocale& locale,
				const std::string& key,
				::uint64_t amount) override;

private:
  std::vector<std::unique_ptr<WMessageResources> > messageResources_;
};

}

#endif // WMESSAGE_RESOURCE_BUNDLE_
