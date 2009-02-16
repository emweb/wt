/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include "CgiParser.h"

#include "Wt/WApplication"
#include "Wt/WLogger"
#include "Wt/WEvent"

namespace {
  using namespace Wt;

  int parseIntEntry(const CgiParser& parser, const std::string& name,
		    int ifMissing) {
    CgiEntry *e;

    if ((e = parser.getEntry(name))) {
      try {
	return boost::lexical_cast<int>(e->value());
      } catch (boost::bad_lexical_cast) {
	wApp->log("error") << "Could not cast event property '" << name 
			   << ": " << e->value() << "' to int";
	return ifMissing;
      }
    } else
      return ifMissing;
  }

  std::string getStringEntry(const CgiParser& parser, const std::string& name) {
    CgiEntry *e;

    if ((e = parser.getEntry(name))) {
      return e->value();
    } else
      return std::string();
  }
}

namespace Wt {

JavaScriptEvent::JavaScriptEvent()
{ }

void JavaScriptEvent::get(const CgiParser& parser, const std::string& se)
{
  type = getStringEntry(parser, se + "type");
  boost::to_lower(type);

  clientX = parseIntEntry(parser, se + "clientX", 0);
  clientY = parseIntEntry(parser, se + "clientY", 0);
  documentX = parseIntEntry(parser, se + "documentX", 0);
  documentY = parseIntEntry(parser, se + "documentY", 0);
  screenX = parseIntEntry(parser, se + "screenX", 0);
  screenY = parseIntEntry(parser, se + "screenY", 0);
  widgetX = parseIntEntry(parser, se + "widgetX", 0);
  widgetY = parseIntEntry(parser, se + "widgetY", 0);
  dragDX = parseIntEntry(parser, se + "dragdX", 0);
  dragDY = parseIntEntry(parser, se + "dragdY", 0);

  modifiers = 0;
  if (parser.getEntry(se + "altKey") != 0)
    modifiers |= AltModifier;

  if (parser.getEntry(se + "ctrlKey") != 0)
    modifiers |= ControlModifier;

  if (parser.getEntry(se + "shiftKey") != 0)
    modifiers |= ShiftModifier;

  if (parser.getEntry(se + "metaKey") != 0)
    modifiers |= MetaModifier;

  keyCode = parseIntEntry(parser, se + "keyCode", 0);
  charCode = parseIntEntry(parser, se + "charCode", 0);

  CgiEntry *e;
  right = (e = parser.getEntry(se + "right")) ? (e->value() == "true") : false;

  scrollX = parseIntEntry(parser, se + "scrollX", 0);
  scrollY = parseIntEntry(parser, se + "scrollY", 0);
  viewportWidth = parseIntEntry(parser, se + "width", 0);
  viewportHeight = parseIntEntry(parser, se + "height", 0);

  response = getStringEntry(parser, se + "response");

  int uean = parseIntEntry(parser, se + "an", 0);
  userEventArgs.clear();
  for (int i = 0; i < uean; ++i) {
    userEventArgs.push_back
      (getStringEntry(parser, se + "a" + boost::lexical_cast<std::string>(i)));
  }
}

WMouseEvent::WMouseEvent()
{ }

WMouseEvent::WMouseEvent(const JavaScriptEvent& jsEvent)
  : jsEvent_(jsEvent)
{ }

WKeyEvent::WKeyEvent()
{ }

WKeyEvent::WKeyEvent(const JavaScriptEvent& jsEvent)
  : jsEvent_(jsEvent)
{ }

Key WKeyEvent::key() const
{
  int key = jsEvent_.keyCode;

  if (key == 0)
    key = jsEvent_.charCode;

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
}

int WKeyEvent::charCode() const
{
  return jsEvent_.charCode ? jsEvent_.charCode : jsEvent_.keyCode;
}

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

WDropEvent::WDropEvent(WObject *source, const std::string& mimeType,
		       const WMouseEvent& mouseEvent)
  : dropSource_(source),
    dropMimeType_(mimeType),
    mouseEvent_(mouseEvent)
{ }

WScrollEvent::WScrollEvent(const JavaScriptEvent& jsEvent)
  : jsEvent_(jsEvent)
{ }

WResponseEvent::WResponseEvent(const JavaScriptEvent& jsEvent)
  : jsEvent_(jsEvent)
{ }

}
