/*
 * Copyright (C) 2026 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WTextF.h"

#include "Wt/WLogger.h"
#include "Wt/WStringStream.h"

#include "Wt/Json/Array.h"
#include "Wt/Json/Value.h"

namespace Wt {
LOGGER("WTextF");

WTextF::WTextF()
{ }

WTextF::WTextF(const WString &text)
  : text_(text)
{ }

WTextF::WTextF(const char *text)
  : text_(text)
{ }

WTextF::WTextF(const std::string &text)
  : text_(text)
{ }

WTextF::WTextF(const WTextF &other)
  : WJavaScriptExposableObject(other),
    text_(other.text())
{ }

WTextF& WTextF::operator=(const WTextF& other)
{
  WJavaScriptExposableObject::operator=(other);

  text_ = other.text_;

  return *this;
}

bool WTextF::operator==(const WTextF& other) const
{
  if (!sameBindingAs(other)) {
    return false;
  }

  return text_ == other.text_;
}

bool WTextF::operator!=(const WTextF& other) const
{
  return !(*this == other);
}


void WTextF::setText(const WString &text)
{
  checkModifiable();
  text_ = text;
}

std::string WTextF::jsValue() const
{
  return text_.jsStringLiteral();
}

WTextF::operator WString() const
{
  checkModifiable();
  return text();
}

void WTextF::assignFromJSON(const Json::Value &value)
{
  try {
#ifndef WT_TARGET_JAVA
    text_ = value;
#else
    text_ = static_cast<WString>(value);
#endif

  } catch (std::exception &e) {
    LOG_ERROR("Couldn't convert JSON to WTextF: " + std::string(e.what()));
  }
}

}