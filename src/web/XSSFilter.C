/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <mxml.h>

#include "Wt/WApplication"
#include "Wt/WLogger"
#include "Wt/WString"

#include "DomElement.h"
#include "XSSUtils.h"

namespace {

class MyHandler
{
public:
  MyHandler();

  static void sax_cb(mxml_node_t *node, mxml_sax_event_t event, void *data);

private:
  void saxCallback(mxml_node_t *node, mxml_sax_event_t event);

  int discard_;
};

MyHandler::MyHandler()
  : discard_(0)
{ }


void MyHandler::saxCallback(mxml_node_t *node, mxml_sax_event_t event)
{
  if (event == MXML_SAX_ELEMENT_OPEN) {
    const char *name = node->value.element.name;

    if (Wt::XSS::isBadTag(name)) {
      wApp->log("warn") << "(XSS) discarding invalid tag: " << name;
      ++discard_;
    }

    if (discard_ == 0) {
      for (int i = 0; i < node->value.element.num_attrs; ++i) {
	const char *aname = node->value.element.attrs[i].name;
	char *v = node->value.element.attrs[i].value;

	if (Wt::XSS::isBadAttribute(aname) || Wt::XSS::isBadAttributeValue(aname, v)) {
	  wApp->log("warn") << "(XSS) discarding invalid attribute: "
			    << aname << ": " << v;
	  mxmlElementDeleteAttr(node, aname);
	  --i;
	}
      }
    }
  } else if (event == MXML_SAX_ELEMENT_CLOSE) {
    std::string name = node->value.element.name;
    if (Wt::XSS::isBadTag(name))
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
    = mxmlSAXLoadString(top, result.c_str(), MXML_OPAQUE_CALLBACK,
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
