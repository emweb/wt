/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WException"
#include "Wt/WLocalizedStrings"
#include "Wt/WMessageResources"

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
  throw WException("WLocalizedStrings::resolvePluralKey is not supported");
}

int WLocalizedStrings::evaluatePluralExpression(const std::string& expression,
						::uint64_t n)
{
  return WMessageResources::evalPluralCase(expression, n);
}

#else
  //TODO
#endif

}
