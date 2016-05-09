/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WJavaScript"
#include <Wt/WWidget>

namespace Wt {
#ifndef WT_CNOR
JSignal<void>::JSignal(WObject *object, const std::string& name,
		       bool collectSlotJavaScript)
  : JSignal<>(object, name, collectSlotJavaScript)
{
}
#endif

void addSignalToWidget(WObject* object, EventSignalBase* signal) {
  WWidget* w = dynamic_cast<WWidget*>(object);
  if(w)
    w->addJSignal(signal);
  
}

}
