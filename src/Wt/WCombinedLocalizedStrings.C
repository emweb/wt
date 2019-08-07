/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WCombinedLocalizedStrings.h"

#include "WebUtils.h"

namespace Wt {

WCombinedLocalizedStrings::WCombinedLocalizedStrings()
{ }

WCombinedLocalizedStrings::~WCombinedLocalizedStrings()
{ }

void WCombinedLocalizedStrings
::add(const std::shared_ptr<WLocalizedStrings>& resolver)
{
  insert(localizedStrings_.size(), resolver);
}

void WCombinedLocalizedStrings
::insert(int index, const std::shared_ptr<WLocalizedStrings>& resolver)
{
  localizedStrings_.insert(localizedStrings_.begin() + index, resolver);
}

void WCombinedLocalizedStrings
::remove(const std::shared_ptr<WLocalizedStrings>& resolver)
{
  Utils::erase(localizedStrings_, resolver);
}

const std::vector<std::shared_ptr<WLocalizedStrings> > &
WCombinedLocalizedStrings::items() const
{
  return localizedStrings_;
}

LocalizedString WCombinedLocalizedStrings::resolveKey(const WLocale& locale,
                                           const std::string& key)
{
  for (unsigned i = 0; i < localizedStrings_.size(); ++i) {
    LocalizedString result = localizedStrings_[i]->resolveKey(locale, key);
    if (result.success)
      return result;
  }

  return LocalizedString();
}

LocalizedString WCombinedLocalizedStrings::resolvePluralKey(const WLocale& locale,
						 const std::string& key,
						 ::uint64_t amount)
{
  for (unsigned i = 0; i < localizedStrings_.size(); ++i) {
    LocalizedString result = localizedStrings_[i]->resolvePluralKey(locale, key, amount);
    if (result.success)
      return result;
  }

  return LocalizedString();
}

void WCombinedLocalizedStrings::hibernate()
{
  for (unsigned i = 0; i < localizedStrings_.size(); ++i)
    localizedStrings_[i]->hibernate();
}

}
