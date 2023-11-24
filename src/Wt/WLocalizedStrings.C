/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
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

LocalizedString WLocalizedStrings::resolvePluralKey(WT_MAYBE_UNUSED const WLocale& locale,
                                                    WT_MAYBE_UNUSED const std::string& key,
                                                    WT_MAYBE_UNUSED ::uint64_t amount)
{
  throw WException("WLocalizedStrings::resolvePluralKey is not supported");
}

int WLocalizedStrings::evaluatePluralExpression(const std::string& expression,
                                                ::uint64_t n)
{
  return WMessageResources::evalPluralCase(expression, n);
}

}
