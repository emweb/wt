/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WCombinedLocalizedStrings"

#include "WebUtils.h"

namespace Wt {

WCombinedLocalizedStrings::WCombinedLocalizedStrings()
{ }

WCombinedLocalizedStrings::~WCombinedLocalizedStrings()
{
  for (unsigned i = 0; i < localizedStrings_.size(); ++i)
    delete localizedStrings_[i];
}

void WCombinedLocalizedStrings::add(WLocalizedStrings* resolver)
{
  localizedStrings_.push_back(resolver);
}

void WCombinedLocalizedStrings::insert(int index, WLocalizedStrings* resolver)
{
  localizedStrings_.insert(localizedStrings_.begin() + index, resolver);
}

void WCombinedLocalizedStrings::remove(WLocalizedStrings *resolver)
{
  Utils::erase(localizedStrings_, resolver);
}

const std::vector<WLocalizedStrings *> &
WCombinedLocalizedStrings::items() const
{
  return localizedStrings_;
}

#ifndef WT_TARGET_JAVA
bool WCombinedLocalizedStrings::resolveKey(const std::string& key,
					   std::string& result)
{
  for (unsigned i = 0; i < localizedStrings_.size(); ++i) {
    if (localizedStrings_[i]->resolveKey(key, result))
      return true;
  }

  return false;
}
#else
std::string *WCombinedLocalizedStrings::resolveKey(const std::string& key)
{
  std::string *result = 0; 

  for (unsigned i = 0; i < localizedStrings_.size(); ++i) {
    result = localizedStrings_[i]->resolveKey(key);
    if (result)
      return result;
  }

  return 0;
}
#endif // WT_TARGET_JAVA

#ifndef WT_TARGET_JAVA
bool WCombinedLocalizedStrings::resolvePluralKey(const std::string& key,
						 std::string& result,
						 ::uint64_t amount)
{
  for (unsigned i = 0; i < localizedStrings_.size(); ++i) {
    if (localizedStrings_[i]->resolvePluralKey(key, result, amount))
      return true;
  }

  return false;
}
#else
  //TODO
#endif // WT_TARGET_JAVA

void WCombinedLocalizedStrings::refresh()
{
  for (unsigned i = 0; i < localizedStrings_.size(); ++i) {
    localizedStrings_[i]->refresh();
  }
}

void WCombinedLocalizedStrings::hibernate()
{
  for (unsigned i = 0; i < localizedStrings_.size(); ++i)
    localizedStrings_[i]->hibernate();
}

}
