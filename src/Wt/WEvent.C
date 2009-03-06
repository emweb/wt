/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include "Wt/WApplication"
#include "Wt/WLogger"
#include "Wt/WEvent"

#include "WebRequest.h"

namespace {
  using namespace Wt;

  int parseIntParameter(const WebRequest& request, const std::string& name,
			int ifMissing) {
    const std::string *p;

    if ((p = request.getParameter(name))) {
      try {
	return boost::lexical_cast<int>(*p);
      } catch (const boost::bad_lexical_cast& ee) {
	wApp->log("error") << "Could not cast event property '" << name 
			   << ": " << *p << "' to int";
	return ifMissing;
      }
    } else
      return ifMissing;
  }

  std::string getStringParameter(const WebRequest& request,
				 const std::string& name) {
    const std::string *p;

    if ((p = request.getParameter(name))) {
      return *p;
    } else
      return std::string();
  }
}

namespace Wt {

JavaScriptEvent::JavaScriptEvent()
{ }

void JavaScriptEvent::get(const WebRequest& request, const std::string& se)
{
  type = getStringParameter(request, se + "type");
  boost::to_lower(type);

  clientX = parseIntParameter(request, se + "clientX", 0);
  clientY = parseIntParameter(request, se + "clientY", 0);
  documentX = parseIntParameter(request, se + "documentX", 0);
  documentY = parseIntParameter(request, se + "documentY", 0);
  screenX = parseIntParameter(request, se + "screenX", 0);
  screenY = parseIntParameter(request, se + "screenY", 0);
  widgetX = parseIntParameter(request, se + "widgetX", 0);
  widgetY = parseIntParameter(request, se + "widgetY", 0);
  dragDX = parseIntParameter(request, se + "dragdX", 0);
  dragDY = parseIntParameter(request, se + "dragdY", 0);

  modifiers = 0;
  if (request.getParameter(se + "altKey") != 0)
    modifiers |= AltModifier;

  if (request.getParameter(se + "ctrlKey") != 0)
    modifiers |= ControlModifier;

  if (request.getParameter(se + "shiftKey") != 0)
    modifiers |= ShiftModifier;

  if (request.getParameter(se + "metaKey") != 0)
    modifiers |= MetaModifier;

  keyCode = parseIntParameter(request, se + "keyCode", 0);
  charCode = parseIntParameter(request, se + "charCode", 0);

  const std::string *p;
  right = (p = request.getParameter(se + "right")) ? (*p == "true") : false;

  scrollX = parseIntParameter(request, se + "scrollX", 0);
  scrollY = parseIntParameter(request, se + "scrollY", 0);
  viewportWidth = parseIntParameter(request, se + "width", 0);
  viewportHeight = parseIntParameter(request, se + "height", 0);

  response = getStringParameter(request, se + "response");

  int uean = parseIntParameter(request, se + "an", 0);
  userEventArgs.clear();
  for (int i = 0; i < uean; ++i) {
    userEventArgs.push_back
      (getStringParameter(request, se + "a"
			  + boost::lexical_cast<std::string>(i)));
  }
}

WMouseEvent::WMouseEvent()
{ }

WMouseEvent::WMouseEvent(const JavaScriptEvent& jsEvent)
  : jsEvent_(jsEvent)
{ }

#ifdef WT_TARGET_JAVA
WMouseEvent WMouseEvent::templateEvent;
#endif // WT_TARGET_JAVA;

WKeyEvent::WKeyEvent()
{ }

WKeyEvent::WKeyEvent(const JavaScriptEvent& jsEvent)
  : jsEvent_(jsEvent)
{ }

#ifdef WT_TARGET_JAVA
WKeyEvent WKeyEvent::templateEvent;

extern Key keyFromValue(int key);
#endif // WT_TARGET_JAVA

Key WKeyEvent::key() const
{
  int key = jsEvent_.keyCode;

  if (key == 0)
    key = jsEvent_.charCode;

#ifndef WT_TARGET_JAVA
  if (key >= 'a' && key <= 'z')
    key -= ('a' - 'A');

  if (key >= 'A' && key <= 'Z')
    return static_cast<Key>(key);
  else if (key == 8 || key == 9 || key == 13 || key == 27 || key == 32
	   || (key >= 16 && key <= 18)
	   || (key >= 33 && key <= 40)
	   || (key >= 45 && key <= 46))
    return static_cast<Key>(key);
  else
    return Key_unknown;
#else // WT_TARGET_JAVA
  return keyFromValue(key);
#endif // WT_TARGET_JAVA

}

int WKeyEvent::charCode() const
{
  return jsEvent_.charCode ? jsEvent_.charCode : jsEvent_.keyCode;
}

#ifndef WT_TARGET_JAVA
WString WKeyEvent::text() const
{
  int c = charCode();
  if (c != 0) {
    wchar_t buf[2];
    buf[0] = charCode();
    buf[1] = 0;
    return WString(buf);
  } else
    return WString();
}
#else // WT_TARGET_JAVA
std::string WKeyEvent::text() const
{
  return std::string() + (char)charCode();
}
#endif // WT_TARGET_JAVA

WDropEvent::WDropEvent(WObject *source, const std::string& mimeType,
		       const WMouseEvent& mouseEvent)
  : dropSource_(source),
    dropMimeType_(mimeType),
    mouseEvent_(mouseEvent)
{ }

WScrollEvent::WScrollEvent()
{ }

WScrollEvent::WScrollEvent(const JavaScriptEvent& jsEvent)
  : jsEvent_(jsEvent)
{ }

#ifdef WT_TARGET_JAVA
WScrollEvent WScrollEvent::templateEvent;
#endif // WT_TARGET_JAVA

}
