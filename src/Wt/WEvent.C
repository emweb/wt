/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include "Wt/WLogger"
#include "Wt/WEvent"

#include "WebRequest.h"
#include "WebSession.h"
#include "3rdparty/rapidxml/rapidxml.hpp"

namespace Wt {
  LOGGER("WEvent");
}

namespace {
  using namespace Wt;

#ifndef WT_TARGET_JAVA
  const std::string& concat(std::string& prefix, int prefixLength, const char *s2)
  {
    prefix.resize(prefixLength);
    prefix += s2;
    return prefix;
  } 
#else
  std::string concat(const std::string& prefix, int prefixLength, const char *s2)
  {
    return prefix + s2;
  }
#endif

  int asInt(const std::string& v) {
    return boost::lexical_cast<int>(v);
  }

  unsigned asUInt(const std::string& v) {
    return boost::lexical_cast<unsigned>(v);
  }

  int parseIntParameter(const WebRequest& request, const std::string& name,
			int ifMissing) {
    const std::string *p;

    if ((p = request.getParameter(name))) {
      try {
	return asInt(*p);
      } catch (const boost::bad_lexical_cast& ee) {
	LOG_ERROR("Could not cast event property '" << name 
		  << ": " << *p << "' to int");
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
      LOG_ERROR("Could not parse touches array '" << str << "'");
      return;
    }

    try {
      for (unsigned i = 0; i < s.size(); i += 9) {
	result.push_back(Touch(asUInt(s[i + 0]),
			       asInt(s[i + 1]), asInt(s[i + 2]),
			       asInt(s[i + 3]), asInt(s[i + 4]),
			       asInt(s[i + 5]), asInt(s[i + 6]),
			       asInt(s[i + 7]), asInt(s[i + 8])));
      }
    } catch (const boost::bad_lexical_cast& ee) {
      LOG_ERROR("Could not parse touches array '" << str << "'");
      return;
    }
  }
}

namespace Wt {

EventType WEvent::eventType() const 
{
  if (!impl_.handler)
    return OtherEvent;

  return impl_.handler->session()->getEventType(*this);
}

Touch::Touch(unsigned identifier,
	     int clientX, int clientY,
	     int documentX, int documentY,
	     int screenX, int screenY,
	     int widgetX, int widgetY)
  : clientX_(clientX),
    clientY_(clientY),
    documentX_(documentX),
    documentY_(documentY),
    screenX_(screenX),
    screenY_(screenY),
    widgetX_(widgetX),
    widgetY_(widgetY),
    identifier_(identifier)
{ }

JavaScriptEvent::JavaScriptEvent()
{ }

void JavaScriptEvent::get(const WebRequest& request, const std::string& se)
{
  std::string s = se;
  int seLength = se.length();

  type = getStringParameter(request, concat(s, seLength, "type"));
  boost::to_lower(type);

  clientX = parseIntParameter(request, concat(s, seLength, "clientX"), 0);
  clientY = parseIntParameter(request, concat(s, seLength, "clientY"), 0);
  documentX = parseIntParameter(request, concat(s, seLength, "documentX"), 0);
  documentY = parseIntParameter(request, concat(s, seLength, "documentY"), 0);
  screenX = parseIntParameter(request, concat(s, seLength, "screenX"), 0);
  screenY = parseIntParameter(request, concat(s, seLength, "screenY"), 0);
  widgetX = parseIntParameter(request, concat(s, seLength, "widgetX"), 0);
  widgetY = parseIntParameter(request, concat(s, seLength, "widgetY"), 0);
  dragDX = parseIntParameter(request, concat(s, seLength, "dragdX"), 0);
  dragDY = parseIntParameter(request, concat(s, seLength, "dragdY"), 0);
  wheelDelta = parseIntParameter(request, concat(s, seLength, "wheel"), 0);

  /*
  if (widgetX == 0 && widgetY == 0) {
    const int signalLength = 7 + se.length();
    const Http::ParameterMap& entries = request.getParameterMap();

    for (Http::ParameterMap::const_iterator i = entries.begin();
	 i != entries.end(); ++i) {
      std::string name = i->first;

      if (name.substr(0, signalLength) == concat(s, seLength, "signal=") {
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
  if (request.getParameter(concat(s, seLength, "altKey")) != 0)
    modifiers |= AltModifier;

  if (request.getParameter(concat(s, seLength, "ctrlKey")) != 0)
    modifiers |= ControlModifier;

  if (request.getParameter(concat(s, seLength, "shiftKey")) != 0)
    modifiers |= ShiftModifier;

  if (request.getParameter(concat(s, seLength, "metaKey")) != 0)
    modifiers |= MetaModifier;

  keyCode = parseIntParameter(request, concat(s, seLength, "keyCode"), 0);
  charCode = parseIntParameter(request, concat(s, seLength, "charCode"), 0);

  button = parseIntParameter(request, concat(s, seLength, "button"), 0);

  scrollX = parseIntParameter(request, concat(s, seLength, "scrollX"), 0);
  scrollY = parseIntParameter(request, concat(s, seLength, "scrollY"), 0);
  viewportWidth = parseIntParameter(request, concat(s, seLength, "width"), 0);
  viewportHeight = parseIntParameter(request, concat(s, seLength, "height"), 0);

  response = getStringParameter(request, concat(s, seLength, "response"));

  int uean = parseIntParameter(request, concat(s, seLength, "an"), 0);
  userEventArgs.clear();
  for (int i = 0; i < uean; ++i) {
    userEventArgs.push_back
      (getStringParameter(request,
			  se + "a" + boost::lexical_cast<std::string>(i)));
  }

  decodeTouches(getStringParameter(request, concat(s, seLength, "touches")),
				   touches);
  decodeTouches(getStringParameter(request, concat(s, seLength, "ttouches")),
				   targetTouches);
  decodeTouches(getStringParameter(request, concat(s, seLength, "ctouches")),
				   changedTouches);  
}

WMouseEvent::WMouseEvent()
{ }

WMouseEvent::WMouseEvent(const JavaScriptEvent& jsEvent)
  : jsEvent_(jsEvent)
{ }

WMouseEvent::Button WMouseEvent::button() const
{
  switch (jsEvent_.button) {
  case 1: return LeftButton;
  case 2: return MiddleButton;
  case 4: return RightButton;
  default: return NoButton;
  }
}

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
  if (key >= 96 && key <= 105)
      key -= int('0');

  if (key >= 'A' && key <= 'Z')
    return static_cast<Key>(key);
  else if (key == 8 || key == 9 || key == 13 || key == 27 || key == 32
	   || (key >= 16 && key <= 18)
	   || (key >= 33 && key <= 40)
	   || (key >= 45 && key <= 46)
       || (key >= 48 && key <= 57)
       || (key >= 112 && key <= 123))
    return static_cast<Key>(key);
  else
    return Key_unknown;
#else // WT_TARGET_JAVA
  return keyFromValue(key);
#endif // WT_TARGET_JAVA

}

int WKeyEvent::charCode() const
{
  return jsEvent_.charCode;
}

#ifndef WT_TARGET_JAVA
WString WKeyEvent::text() const
{
  int c = charCode();
  if (c != 0) {
    char buf[10]; // 4 is enough
    char *ptr = buf;
    try {
      Wt::rapidxml::xml_document<>::insert_coded_character<0>(ptr, charCode());
    } catch (Wt::rapidxml::parse_error& e) {
      LOG_ERROR("charcode: " << e.what());
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
