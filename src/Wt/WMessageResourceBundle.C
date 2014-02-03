/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WMessageResourceBundle"
#include "Wt/WMessageResources"

namespace Wt {

WMessageResourceBundle::WMessageResourceBundle()
{
}

WMessageResourceBundle::~WMessageResourceBundle()
{
  for (unsigned i = 0; i < messageResources_.size(); ++i)
    delete messageResources_[i];
}

void WMessageResourceBundle::use(const std::string& path, bool loadInMemory)
{
  for (unsigned i = 0; i < messageResources_.size(); ++i)
    if ((!messageResources_[i]->path().empty()) &&
        messageResources_[i]->path() == path)
      return;

  messageResources_.push_back(new WMessageResources(path, loadInMemory));
}

void WMessageResourceBundle::useBuiltin(const char *xmlbundle)
{
  for (unsigned i = 0; i < messageResources_.size(); ++i)
    if (messageResources_[i]->isBuiltin(xmlbundle))
      return;

  messageResources_.insert(messageResources_.begin(),
			   new WMessageResources(xmlbundle));
}

#ifndef WT_TARGET_JAVA
bool WMessageResourceBundle::resolveKey(const std::string& key,
					std::string& result)
{
  for (unsigned i = 0; i < messageResources_.size(); ++i) {
    if (messageResources_[i]->resolveKey(key, result))
      return true;
  }

  return false;
}
#else
std::string *WMessageResourceBundle::resolveKey(const std::string& key)
{
  return 0;
}
#endif // WT_TARGET_JAVA

#ifndef WT_TARGET_JAVA
bool WMessageResourceBundle::resolvePluralKey(const std::string& key,
					      std::string& result,
					      ::uint64_t amount)
{
  for (unsigned i = 0; i < messageResources_.size(); ++i) {
    if (messageResources_[i]->resolvePluralKey(key, result, amount))
      return true;
  }

  return false;
}
#else
std::string *WMessageResourceBundle::resolvePluralKey(const std::string& key,
						      ::uint64_t amount)
{
  return 0;
}
#endif // WT_TARGET_JAVA

void WMessageResourceBundle::refresh()
{
  for (unsigned i = 0; i < messageResources_.size(); ++i) {
    messageResources_[i]->refresh();
  }
}

void WMessageResourceBundle::hibernate()
{
  for (unsigned i = 0; i < messageResources_.size(); ++i)
    messageResources_[i]->hibernate();
}

const std::set<std::string> 
WMessageResourceBundle::keys(WFlags<Scope> scope) const
{
  std::set<std::string> keys;

  for (unsigned i = 0; i < messageResources_.size(); ++i) {
    const std::set<std::string>& resources = messageResources_[i]->keys(scope);
    keys.insert(resources.begin(), resources.end());
  }

  return keys;
}

}
