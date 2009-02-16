/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WMessageResourceBundle"
#include "Wt/WMessageResources"
#include "Wt/WString"

namespace Wt {

WMessageResourceBundle::WMessageResourceBundle()
{ }

WMessageResourceBundle::~WMessageResourceBundle()
{
  for (unsigned i = 0; i < messageResources_.size(); ++i)
    delete messageResources_[i];
}

void WMessageResourceBundle::use(const std::string& path, bool loadInMemory)
{
  for (unsigned i = 0; i < messageResources_.size(); ++i)
    if (messageResources_[i]->path() == path)
      return;

  messageResources_.push_back(new WMessageResources(path, loadInMemory));
}

#ifndef WT_TARGET_JAVA
bool WMessageResourceBundle::resolveKey(const std::string& key,
					std::string& result)
{
  for (unsigned i = 0; i < messageResources_.size(); ++i) {
    if (messageResources_[i]->resolveKey(key, result))
      return true;
  }

  result = "??" + key + "??";
  return false;
}
#else
std::string *WMessageResourceBundle::resolveKey(const std::string& key)
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

}
