/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WLocalizedStrings"
#include "Wt/WString"

namespace Wt {

WLocalizedStrings::~WLocalizedStrings()
{ }

std::string WLocalizedStrings::getUTF8Value(const WString& s)
{
  return s.toUTF8();
}

std::wstring WLocalizedStrings::getValue(const WString& s)
{
  return s.value();
}

void WLocalizedStrings::refresh()
{ }

void WLocalizedStrings::hibernate()
{ }

}
