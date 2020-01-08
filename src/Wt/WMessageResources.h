// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WMESSAGE_RESOURCES_
#define WMESSAGE_RESOURCES_

#include <string>
#include <vector>
#include <map>
#include <set>
#include <Wt/WFlags.h>
#include <Wt/WMessageResourceBundle.h>
#include <Wt/WDllDefs.h>

#ifdef WT_THREADED
#include <mutex>
#endif

namespace Wt {

class WString;

class WT_API WMessageResources
{
public:
  WMessageResources(const std::string& path, bool loadInMemory = true);
  WMessageResources(const char *builtin);

  void hibernate();

  bool isBuiltin(const char *data) const { return builtin_ == data; }
  const std::string& path() const { return path_; }

  LocalizedString resolveKey(const WLocale& locale, const std::string& key) const;
  LocalizedString resolvePluralKey(const WLocale& locale,
			const std::string& key, 
			::uint64_t amount) const;

  static int evalPluralCase(const std::string &expression, ::uint64_t n);

  std::set<std::string> keys(const WLocale& locale) const;

private:
  typedef std::map<std::string, std::vector<std::string> > KeyValuesMap;

  struct Resource {
    KeyValuesMap map_;
    std::string pluralExpression_;
    unsigned pluralCount_;
  };

  typedef std::map<std::string, Resource> ResourceMap;

  bool loadInMemory_;
  std::string path_;
  const char *builtin_;
#ifdef WT_THREADED
  std::mutex resourceMutex_;
#endif
  mutable ResourceMap resources_;

  void load(const WLocale& locale) const;
  LocalizedString resolve(const std::string& locale, const std::string& key) const;
  LocalizedString resolvePlural(const std::string& locale, const std::string& key, ::uint64_t amount) const;
  bool readResourceFile(const std::string& locale, Resource& resource) const;
  bool readResourceStream(std::istream &s, Resource& resource,
                          const std::string &fileName) const;

  std::string findCase(const std::vector<std::string> &cases,
		       std::string pluralExpression,
		       ::uint64_t amount) const;
};

}

#endif // WMESSAGE_RESOURCES_
