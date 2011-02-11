/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WLocalizedStrings"
#include "Wt/WString"

#include <stdexcept>

namespace Wt {

WLocalizedStrings::~WLocalizedStrings()
{ }

void WLocalizedStrings::refresh()
{ }

void WLocalizedStrings::hibernate()
{ }

#ifndef WT_TARGET_JAVA
bool WLocalizedStrings::resolvePluralKey(const std::string& key, 
					 std::string& result, 
					 ::uint64_t amount)
{
  throw 
    std::logic_error("WLocalizedStrings::resolvePluralKey is not supported");
}
#else
  //TODO
#endif

}
