/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_CNOR
#include <fstream>

#include <boost/lexical_cast.hpp>

#include "DomElement.h"
#include "Wt/WLogger"
#include "Wt/WMessageResources"
#include "Wt/WString"
#include "Wt/WApplication"

#include <mxml.h>

namespace {

using namespace Wt;

/*
 * Mini-XML SAX-like handler for reading a resource file
 */
class WMessageHandler
{
public:
  WMessageHandler(WMessageResources::KeyValueMap& valueMap);

  static void sax_cb(mxml_node_t *node, mxml_sax_event_t event, void *data);

private:
  WMessageResources::KeyValueMap& valueMap_;

  void saxCallback(mxml_node_t *node, mxml_sax_event_t event);

  int depth_;
  std::string currentKey_;
  mxml_node_t *currentMessage_;
  bool inMessage_;
};

WMessageHandler::WMessageHandler(WMessageResources::KeyValueMap& valueMap)
  : valueMap_(valueMap),
    depth_(0),
    currentMessage_(0),
    inMessage_(false)
{ }

void WMessageHandler::saxCallback(mxml_node_t *node, mxml_sax_event_t event)
{
  if (event == MXML_SAX_ELEMENT_OPEN) {
    ++depth_;

    if (!inMessage_ && depth_ == 2) {
      char *name = node->value.element.name;

      if (!strcmp(name, "message")) {
	currentKey_.clear();

	const char *id = mxmlElementGetAttr(node, "id");

	if (id) {
	  currentKey_ = id;
	  inMessage_ = true;
	  if (currentMessage_)
	    mxmlDelete(currentMessage_);
	  currentMessage_ = node;
	}
      }
    }
  } else if (event == MXML_SAX_ELEMENT_CLOSE) {
    if (depth_ == 2 && inMessage_) {
      if (currentMessage_->child) {
	char *currentValue
	  = mxmlSaveAllocString(currentMessage_->child, MXML_NO_CALLBACK);

	if (currentValue) {
	  std::string v(currentValue);
	  valueMap_[currentKey_] = v.substr(0, v.length()-1);

	  free(currentValue);
	} else
	  wApp->log("error") << "Wt internal error: mxmlSaveAllocString() "
	    "failed.";
      } else {
	valueMap_[currentKey_] = std::string();
      }

      inMessage_ = false;
    } else {
      std::string name = node->value.element.name;

      if (!node->child && !DomElement::isSelfClosingTag(name)) {
	mxmlNewText(node, 0, "");
      }
    }

    --depth_;
  }

  if (inMessage_) {
    if (event == MXML_SAX_ELEMENT_OPEN)
      mxmlRetain(node);
    else if (event == MXML_SAX_DIRECTIVE)
      mxmlRetain(node);
    else if ((event == MXML_SAX_DATA || event == MXML_SAX_CDATA)
	     && node->parent->ref_count > 1) {
      /*
       * If the parent was retained, then retain
       * this data node as well.
       */
      mxmlRetain(node);
    }
  }
}

void WMessageHandler::sax_cb(mxml_node_t *node, mxml_sax_event_t event,
			     void *data)
{
  WMessageHandler *instance = (WMessageHandler *)data;

  instance->saxCallback(node, event);
}

} // end namespace

namespace Wt {

WMessageResources::WMessageResources(const std::string& path,
				     bool loadInMemory)
  : loadInMemory_(loadInMemory),
    loaded_(false),
    path_(path)
{ }

void WMessageResources::refresh()
{
  defaults_.clear();
  readResourceFile("", defaults_);

  local_.clear();
  std::string locale = wApp->locale();

  if (!locale.empty())
    for(;;) {
      if (readResourceFile(locale, local_))
	break;

      /* try a lesser specified variant */
      std::string::size_type l = locale.rfind('-');
      if (l != std::string::npos)
	locale.erase(l);
      else
	break;
    }

  loaded_ = true;
}

void WMessageResources::hibernate()
{
  if (!loadInMemory_) {
    defaults_.clear();
    local_.clear();
    loaded_ = false;
  }
}

bool WMessageResources::resolveKey(const std::string& key, std::string& result)
{
  if (!loaded_)
    refresh();

  KeyValueMap::const_iterator j;

  j = local_.find(key);
  if (j != local_.end()) {
    result = j->second;
    return true;
  }

  j = defaults_.find(key);
  if (j != defaults_.end()) {
    result = j->second;
    return true;
  }

  return false;
}

bool WMessageResources::readResourceFile(const std::string& locale,
					 KeyValueMap& valueMap)
{
  std::string fileName
    = path_ + (locale.length() > 0 ? "_" : "") + locale + ".xml";

  {
    std::ifstream test(fileName.c_str());
    if (!test)
      return false;
  }

  WMessageHandler handler(valueMap);

  FILE *fp = fopen(fileName.c_str(), "r");

  mxmlSetWrapMargin(0);
  mxml_node_t *top = mxmlNewElement(MXML_NO_PARENT, "top");
  mxml_node_t *first
    = mxmlSAXLoadFile(top, fp, MXML_OPAQUE_CALLBACK, WMessageHandler::sax_cb,
		      &handler);
  if (!first)
    wApp->log("error") << "Error reading " << fileName;

  if (fp)
    fclose(fp);

  mxmlDelete(top);

  return true;
}

}
#endif // WT_CNOR

