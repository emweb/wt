/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WMessageResourceBundle.h"
#include "Wt/WMessageResources.h"

namespace Wt {

WMessageResourceBundle::WMessageResourceBundle()
{ }

WMessageResourceBundle::~WMessageResourceBundle()
{ }

void WMessageResourceBundle::use(const std::string& path, bool loadInMemory)
{
  for (unsigned i = 0; i < messageResources_.size(); ++i)
    if ((!messageResources_[i]->path().empty()) &&
        messageResources_[i]->path() == path)
      return;

  messageResources_.push_back
    (std::unique_ptr<WMessageResources>
     (new WMessageResources(path, loadInMemory)));
}

void WMessageResourceBundle::useBuiltin(const char *xmlbundle)
{
  for (unsigned i = 0; i < messageResources_.size(); ++i)
    if (messageResources_[i]->isBuiltin(xmlbundle))
      return;

  messageResources_.insert
    (messageResources_.begin(),
     std::unique_ptr<WMessageResources>(new WMessageResources(xmlbundle)));
}

LocalizedString WMessageResourceBundle::resolveKey(const WLocale& locale,
                                                   const std::string& key)
{
  for (unsigned i = 0; i < messageResources_.size(); ++i) {
    LocalizedString result = messageResources_[i]->resolveKey(locale, key);
    if (result)
      return result;
  }

  return LocalizedString{};
}

LocalizedString WMessageResourceBundle::resolvePluralKey(const WLocale& locale,
                                                         const std::string& key,
                                                         ::uint64_t amount)
{
  for (unsigned i = 0; i < messageResources_.size(); ++i) {
    LocalizedString result = messageResources_[i]->resolvePluralKey(locale, key, amount);
    if (result)
      return result;
  }

  return LocalizedString{};
}

void WMessageResourceBundle::hibernate()
{
  for (unsigned i = 0; i < messageResources_.size(); ++i)
    messageResources_[i]->hibernate();
}

const std::set<std::string> 
WMessageResourceBundle::keys(const WLocale& locale) const
{
  std::set<std::string> keys;

  for (unsigned i = 0; i < messageResources_.size(); ++i) {
    const std::set<std::string>& resources = messageResources_[i]->keys(locale);
    keys.insert(resources.begin(), resources.end());
  }

  return keys;
}

}
