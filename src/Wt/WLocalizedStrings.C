/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WException.h"
#include "Wt/WLocalizedStrings.h"
#include "Wt/WMessageResources.h"

namespace Wt {

WLocalizedStrings::~WLocalizedStrings()
{ }

void WLocalizedStrings::hibernate()
{ }

LocalizedString WLocalizedStrings::resolvePluralKey(const WLocale& locale,
					 const std::string& key, 
					 ::uint64_t amount)
{
  throw WException("WLocalizedStrings::resolvePluralKey is not supported");
}

int WLocalizedStrings::evaluatePluralExpression(const std::string& expression,
						::uint64_t n)
{
  return WMessageResources::evalPluralCase(expression, n);
}

}
