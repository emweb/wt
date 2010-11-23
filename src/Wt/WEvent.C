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
#include "rapidxml/rapidxml.hpp"

namespace {
  using namespace Wt;

  int asInt(const std::string& v) {
    return boost::lexical_cast<int>(v);
  }

  int parseIntParameter(const WebRequest& request, const std::string& name,
			int ifMissing) {
    const std::string *p;

    if ((p = request.getParameter(name))) {
      try {
	return asInt(*p);
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

  void decodeTouches(std::string str, std::vector<Touch>& result) {
    if (str.empty())
      return;

    std::vector<std::string> s;
    boost::split(s, str, boost::is_any_of(";"));
    
    if (s.size() % 9) {
      wApp->log("error") << "Could not parse touches array '" << str << "'";
      return;
    }

    try {
      for (unsigned i = 0; i < s.size(); i += 9) {
	result.push_back(Touch(asInt(s[i + 0]),
			       asInt(s[i + 1]), asInt(s[i + 2]),
			       asInt(s[i + 3]), asInt(s[i + 4]),
			       asInt(s[i + 5]), asInt(s[i + 6]),
			       asInt(s[i + 7]), asInt(s[i + 8])));
      }
    } catch (const boost::bad_lexical_cast& ee) {
      wApp->log("error") << "Could not parse touches array '" << str << "'";
      return;
    }
  }
}

namespace Wt {

Touch::Touch(int identifier,
	     int clientX, int clientY,
	     int documentX, int documentY,
	     int screenX, int screenY,
	     int widgetX, int widgetY)
  : clientX_(clientX),
    clientY_(clientY),
    documentX_(documentX),
    documentY_(documentY),
    widgetX_(widgetX),
    widgetY_(widgetY),
    identifier_(identifier)
{ }

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
  wheelDelta = parseIntParameter(request, se + "wheel", 0);

  /*
  if (widgetX == 0 && widgetY == 0) {
    const int signalLength = 7 + se.length();
    const Http::ParameterMap& entries = request.getParameterMap();

    for (Http::ParameterMap::const_iterator i = entries.begin();
	 i != entries.end(); ++i) {
      std::string name = i->first;

      if (name.substr(0, signalLength) == se + "signal=") {
	std::string e = name.substr(name.length() - 2);
	if (e == ".x") {
	  try {
	    widgetX = boost::lexical_cast<int>(i->second[0]);
	  } catch (const boost::bad_lexical_cast& ee) {
	  }
	} else if (e == ".y") {
	  try {
	    widgetY = boost::lexical_cast<int>(i->second[0]);
	  } catch (const boost::bad_lexical_cast& ee) {
	  }
	}
      }
    }
  }
  */

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

  button = parseIntParameter(request, se + "button", 0);

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

  decodeTouches(getStringParameter(request, se + "touches"), touches);
  decodeTouches(getStringParameter(request, se + "ttouches"), targetTouches);
  decodeTouches(getStringParameter(request, se + "ctouches"), changedTouches);  
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
    char buf[10]; // 4 is enough
    char *ptr = buf;
    try {
      rapidxml::xml_document<>::insert_coded_character<0>(ptr, charCode());
    } catch (rapidxml::parse_error& e) {
      if (WApplication::instance())
        WApplication::instance()->log("error")
          << "WKeyEvent charcode: " << e.what();
      return WString();
    }
    return WString::fromUTF8(std::string(buf, ptr));
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

WTouchEvent::WTouchEvent()
{ }

WTouchEvent::WTouchEvent(const JavaScriptEvent& jsEvent)
  : jsEvent_(jsEvent)
{ }

#ifdef WT_TARGET_JAVA
WTouchEvent WTouchEvent::templateEvent;
#endif // WT_TARGET_JAVA;

WGestureEvent::WGestureEvent()
{ }

WGestureEvent::WGestureEvent(const JavaScriptEvent& jsEvent)
  : jsEvent_(jsEvent)
{ }

#ifdef WT_TARGET_JAVA
WGestureEvent WGestureEvent::templateEvent;
#endif // WT_TARGET_JAVA;

}
