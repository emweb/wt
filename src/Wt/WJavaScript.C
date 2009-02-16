/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WJavaScript"

#include "WtException.h"

namespace Wt {

JSignal<void>::JSignal(WObject *object, const std::string& name,
		       bool collectSlotJavaScript)
  : JSignal<>(object, name, collectSlotJavaScript)
{ }

void throwWtException(const std::string& msg)
{
  throw WtException(msg);
}

}
