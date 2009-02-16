/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/algorithm/string.hpp>
#include <mxml.h>

#include "Wt/WApplication"
#include "Wt/WLogger"
#include "Wt/WString"

#include "DomElement.h"

namespace {

class MyHandler
{
public:
  MyHandler();

  static void sax_cb(mxml_node_t *node, mxml_sax_event_t event, void *data);

private:
  void saxCallback(mxml_node_t *node, mxml_sax_event_t event);

  bool isBadTag(const std::string& name);
  bool isBadAttribute(const std::string& name);
  bool isBadAttributeValue(const std::string& name, const std::string& value);

  int discard_;
};

MyHandler::MyHandler()
  : discard_(0)
{ }

bool MyHandler::isBadTag(const std::string& name)
{
  return (boost::iequals(name, "script")
	  || boost::iequals(name, "applet")
	  || boost::iequals(name, "object")
	  || boost::iequals(name, "iframe")
	  || boost::iequals(name, "frame")
	  || boost::iequals(name, "layer")
	  || boost::iequals(name, "ilayer")
	  || boost::iequals(name, "frameset")
	  || boost::iequals(name, "link")
	  || boost::iequals(name, "meta")
	  || boost::iequals(name, "title")
	  || boost::iequals(name, "base")
	  || boost::iequals(name, "basefont")
	  || boost::iequals(name, "bgsound")
	  || boost::iequals(name, "head")
	  || boost::iequals(name, "body")
	  || boost::iequals(name, "embed")
	  || boost::iequals(name, "style")
	  || boost::iequals(name, "blink"));
}

bool MyHandler::isBadAttribute(const std::string& name)
{
  return (boost::istarts_with(name, "on")
	  || boost::istarts_with(name, "data")
	  || boost::iequals(name, "dynsrc")
	  || boost::iequals(name, "id")
	  || boost::iequals(name, "name"));
}

bool MyHandler::isBadAttributeValue(const std::string& name,
				    const std::string& value)
{
  if (boost::iequals(name, "action")
      || boost::iequals(name, "background")
      || boost::iequals(name, "codebase")
      || boost::iequals(name, "dynsrc")
      || boost::iequals(name, "href")
      || boost::iequals(name, "src"))
    return (boost::istarts_with(value, "javascript:")
	    || boost::istarts_with(value, "vbscript:")
	    || boost::istarts_with(value, "about:")
	    || boost::istarts_with(value, "chrome:")
	    || boost::istarts_with(value, "data:")
	    || boost::istarts_with(value, "disk:")
	    || boost::istarts_with(value, "hcp:")
	    || boost::istarts_with(value, "help:")
	    || boost::istarts_with(value, "livescript")
	    || boost::istarts_with(value, "lynxcgi:")
	    || boost::istarts_with(value, "lynxexec:")
	    || boost::istarts_with(value, "ms-help:")
	    || boost::istarts_with(value, "ms-its:")
	    || boost::istarts_with(value, "mhtml:")
	    || boost::istarts_with(value, "mocha:")
	    || boost::istarts_with(value, "opera:")
	    || boost::istarts_with(value, "res:")
	    || boost::istarts_with(value, "resource:")
	    || boost::istarts_with(value, "shell:")
	    || boost::istarts_with(value, "view-source:")
	    || boost::istarts_with(value, "vnd.ms.radio:")
	    || boost::istarts_with(value, "wysiwyg:"));
  else
    if (boost::iequals(name, "style"))
      return boost::icontains(value, "absolute")
	|| boost::icontains(value, "behaviour")
	|| boost::icontains(value, "content")
	|| boost::icontains(value, "expression")
	|| boost::icontains(value, "fixed")
	|| boost::icontains(value, "include-source")
	|| boost::icontains(value, "moz-binding")
	|| boost::icontains(value, "javascript");
    else
      return false;
}

void MyHandler::saxCallback(mxml_node_t *node, mxml_sax_event_t event)
{
  if (event == MXML_SAX_ELEMENT_OPEN) {
    const char *name = node->value.element.name;

    if (isBadTag(name)) {
      wApp->log("warn") << "(XSS) discarding invalid tag: " << name;
      ++discard_;
    }

    if (discard_ == 0) {
      for (int i = 0; i < node->value.element.num_attrs; ++i) {
	const char *aname = node->value.element.attrs[i].name;
	char *v = node->value.element.attrs[i].value;
	
	if (isBadAttribute(aname) || isBadAttributeValue(aname, v)) {
	  wApp->log("warn") << "(XSS) discarding invalid attribute: "
			    << aname << ": " << v;
	  mxmlElementDeleteAttr(node, aname);
	  --i;
	}
      }
    }
  } else if (event == MXML_SAX_ELEMENT_CLOSE) {
    std::string name = node->value.element.name;
    if (isBadTag(name))
      --discard_;

    if (!discard_) {
      if (!node->child && !Wt::DomElement::isSelfClosingTag(name)) {
	mxmlNewText(node, 0, "");
      }
    }
  }

  if (!discard_) {
    if (event == MXML_SAX_ELEMENT_OPEN)
      mxmlRetain(node);
    else if (event == MXML_SAX_DIRECTIVE)
      mxmlRetain(node);
    else if (event == MXML_SAX_DATA && node->parent->ref_count > 1) {
      /*
       * If the parent was retained, then retain
       * this data node as well.
       */
      mxmlRetain(node);
    }
  }
}

void MyHandler::sax_cb(mxml_node_t *node, mxml_sax_event_t event,
			     void *data)
{
  MyHandler *instance = (MyHandler *)data;

  instance->saxCallback(node, event);
}

}

namespace Wt {

bool XSSFilterRemoveScript(WString& text)
{
  if (text.empty())
    return true;

  std::string result = "<span>" + text.toUTF8() + "</span>";

  MyHandler handler;

  mxml_node_t *top = mxmlNewElement(MXML_NO_PARENT, "span");
  mxml_node_t *first
    = mxmlSAXLoadString(top, result.c_str(), MXML_NO_CALLBACK,
			MyHandler::sax_cb, &handler);

  if (first) {
    char *r = mxmlSaveAllocString(top, MXML_NO_CALLBACK);
    result = r;
    free(r);
  } else {
    mxmlDelete(top);
    wApp->log("error") << "Error parsing: " << text;

    return false;
  }

  mxmlDelete(top);

  /*
   * 27 is the length of '<span><span>x</span></span>\n'
   */

  if (result.length() < 28)
    result.clear();
  else
    result = result.substr(12, result.length() - 27);

  text = WString::fromUTF8(result);

  return true;
}

}
