/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WJavaScript.h"
#include "Wt/WApplication.h"
#include "Wt/WWidget.h"

namespace Wt {

void addSignalToWidget(WObject* object, EventSignalBase* signal) {
  WWidget* w = dynamic_cast<WWidget*>(object);
  if(w)
    w->addJSignal(signal);
  
}

namespace Impl {

void unMarshal(const JavaScriptEvent &jse, int argi, std::string &s)
{
  if ((unsigned)argi >= jse.userEventArgs.size()) {
    Wt::log("error") << "JSignal: missing JavaScript argument:" << argi;
    return;
  }

  std::string v = jse.userEventArgs[argi];
  WString::checkUTF8Encoding(v);

  s = v;
}

void unMarshal(const JavaScriptEvent& jse, int argi, WString& s) {
  if ((unsigned)argi >= jse.userEventArgs.size()) {
    Wt::log("error") << "JSignal: missing JavaScript argument:" << argi;
    return;
  }

  std::string v = jse.userEventArgs[argi];
  s = WString::fromUTF8(v);
}

void unMarshal(const JavaScriptEvent& jse, int argi, NoClass& nc) {
  if ((unsigned)argi < jse.userEventArgs.size()) {
    Wt::log("error") << "JSignal: redundant JavaScript argument: '"
		     << jse.userEventArgs[argi] << "'";
  }
}

void unMarshal(const JavaScriptEvent& jse, int, WMouseEvent& e) {
  e = WMouseEvent(jse);
}

void unMarshal(const JavaScriptEvent& jse, int, WKeyEvent& e) {
  e = WKeyEvent(jse);
}

void unMarshal(const JavaScriptEvent& jse, int, WTouchEvent& e) {
  e = WTouchEvent(jse);
}

}

std::string senderId(WObject *sender)
{
  if (sender == WApplication::instance())
    return "app";
  else
    return sender->id();
}

}
