// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WCOMBINED_LOCALIZED_STRINGS_
#define WCOMBINED_LOCALIZED_STRINGS_

#include <vector>
#include <Wt/WLocalizedStrings.h>

namespace Wt {

class WMessageResources;

/*! \class WCombinedLocalizedStrings Wt/WCombinedLocalizedStrings.h Wt/WCombinedLocalizedStrings.h
 *  \brief A localized string resolver that bundles multiple string resolvers.
 *
 * This class implements the localized strings interface and delegates
 * WString::tr() string resolution to one or more string
 * resolvers. You will typically use this class if you want to combine
 * different methods of string resolution (e.g. some from files, and
 * other strings using a database).
 *
 * \sa WApplication::setLocalizedStrings()
 */
class WT_API WCombinedLocalizedStrings : public WLocalizedStrings
{
public:
  /*! \brief Constructor.
   */
  WCombinedLocalizedStrings();

  virtual ~WCombinedLocalizedStrings();

  /*! \brief Adds a string resolver.
   *
   * The order in which string resolvers are added is significant:
   * resolveKey() will consult each string resolver in the order they
   * have been added, until a match is found.
   *
   * \if cpp
   * Ownership of the resolver is transferred.
   * \endif
   */
  void add(const std::shared_ptr<WLocalizedStrings>& resolver);

  /*! \brief Inserts a string resolver.
   *
   * \sa add()
   */
  void insert(int index, const std::shared_ptr<WLocalizedStrings>& resolver);

  /*! \brief Removes a string resolver.
   *
   * \sa add()
   */
  void remove(const std::shared_ptr<WLocalizedStrings>& resolver);

  const std::vector<std::shared_ptr<WLocalizedStrings> >& items() const;

  virtual void hibernate() override;

  virtual LocalizedString resolveKey(const WLocale& locale, const std::string& key) override;

  virtual LocalizedString resolvePluralKey(const WLocale& locale, const std::string& key, ::uint64_t amount) override;

private:
  std::vector<std::shared_ptr<WLocalizedStrings> > localizedStrings_;
};

}

#endif // WCOMBINED_LOCALIZED_STRINGS_
