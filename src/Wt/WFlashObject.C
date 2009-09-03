/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WFlashObject"
#include "Wt/WApplication"
#include "DomElement.h"
#include <boost/lexical_cast.hpp>

#include <sstream>

namespace Wt {

WFlashObject::WFlashObject(const std::string& url,
                           WContainerWidget *parent)
  : WContainerWidget(parent),
    url_(url),
    isRendered_(false),
    sizeChanged_(false)
{
  wApp->require(WApplication::resourcesUrl() + "swfobject.js");
}

WFlashObject::~WFlashObject()
{
  wApp->doJavaScript("if (swfobject) {swfobject.removeSWF(flash" + id() + "); }");

}

void WFlashObject::setFlashParameter(const std::string &name,
    const WString &value)
{
  WString v = value;
  parameters_[name] = v;
}

void WFlashObject::setFlashVariable(const std::string &name,
    const WString &value)
{
  WString v = value;
  variables_[name] = v;
}

namespace {
  std::string mapToJsMap(const std::map<std::string, WString> &map)
  {
    std::stringstream ss;
    bool first = true;
    ss << "{";
    for (std::map<std::string, WString>::const_iterator i = map.begin();
      i != map.end(); ++i) {
        if (first) {
          first = false;
        } else {
          ss << ", ";
        }
        ss << i->first << ": " << i->second.jsStringLiteral();
    }
    ss << "}";
    return ss.str();
  }
}

DomElement *WFlashObject::createDomElement(WApplication *app)
{
  DomElement *result = WContainerWidget::createDomElement(app);
  DomElement *innerElement = DomElement::createNew(DomElement_DIV);
  innerElement->setId("flash" + id());
  result->addChild(innerElement);
  std::string flashvars = mapToJsMap(variables_);
  std::string params = mapToJsMap(parameters_);
  std::string attributes = "{}";
  if (styleClass() != "") {
    attributes = "{ styleclass: " + jsStringLiteral(styleClass()) + " }";
  }
  wApp->doJavaScript("if (swfobject) {swfobject.embedSWF(\"" + url_ + "\", \"" +
      "flash" + id() + "\", \"" +
      boost::lexical_cast<std::string>((int)width().toPixels()) + "\", \"" +
      boost::lexical_cast<std::string>((int)height().toPixels()) +
      "\", \"8.0.0\", " +
      "false, " + //expressinstall
      flashvars + ", " +
      params + ", " +
      attributes + ");}");
  isRendered_ = true;
  return result;
}

std::string WFlashObject::jsFlashRef() const
{
  return WT_CLASS ".getElement('flash" + id() + "')";
}

void WFlashObject::getDomChanges(std::vector<DomElement *>& result,
                                 WApplication *app)
{
  WContainerWidget::getDomChanges(result, app);
  if (isRendered_ && sizeChanged_) {
    // Note: innerElement is no longer a DomElement_DIV, but that is not
    // important now
    DomElement *innerElement =
      DomElement::getForUpdate("flash" + id(), DomElement_DIV);
    innerElement->setAttribute("width",
        boost::lexical_cast<std::string>((int)width().toPixels()));
    innerElement->setAttribute("height",
        boost::lexical_cast<std::string>((int)height().toPixels()));
    result.push_back(innerElement);
  }
}

void WFlashObject::resize(const WLength &width, const WLength &height)
{
  if (isRendered_) {
    sizeChanged_ = true;
  }
  WContainerWidget::resize(width, height);
}

}
