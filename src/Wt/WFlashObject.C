/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WFlashObject"
#include "Wt/WAnchor"
#include "Wt/WApplication"
#include "Wt/WEnvironment"
#include "Wt/WImage"
#include "DomElement.h"
#include "Utils.h"

#include <boost/lexical_cast.hpp>
#include <sstream>

namespace Wt {

WFlashObject::WFlashObject(const std::string& url,
                           WContainerWidget *parent)
  : WWebWidget(parent),
    url_(url),
    sizeChanged_(false),
    alternative_(0)
{
  setInline(false);
  setAlternativeContent(new WAnchor("http://www.adobe.com/go/getflashplayer",
    new WImage("http://www.adobe.com/images/"
               "shared/download_buttons/get_flash_player.gif")));
}

WFlashObject::~WFlashObject()
{
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

void WFlashObject::updateDom(DomElement& element, bool all)
{
  if (all) {
    //http://latrine.dgx.cz/how-to-correctly-insert-a-flash-into-xhtml
    DomElement *obj = DomElement::createNew(DomElement_OBJECT);

    if (isInLayout()) {
      // Layout-manager managed sizes need some CSS magic to display
      // correctly
      obj->setProperty(PropertyStylePosition, "absolute");
      obj->setProperty(PropertyStyleLeft, "0");
      obj->setProperty(PropertyStyleRight, "0");
      element.setProperty(PropertyStylePosition, "relative");
      std::stringstream ss;
      // Client-side auto-resize function
      ss <<
        """function(self, w, h) {"
        ""  "v=self.firstChild;"
        ""  "v.setAttribute('width', w);"
        ""  "v.setAttribute('height', h);";
      if (alternative_) {
        ss <<
          """a=v.lastChild;"
          ""  "if(a && a." << WT_RESIZE_JS <<")"
          ""    "a." << WT_RESIZE_JS << "(a, w, h);";
      }
      ss
        <<"}";
      setJavaScriptMember(WT_RESIZE_JS, ss.str());
    }

    obj->setId(id() + "_flash");
    obj->setAttribute("type", "application/x-shockwave-flash");
    if (!wApp->environment().agentIsIE()) {
      obj->setAttribute("data", url_);
    }
    element.setAttribute("width", width().isAuto() ? "" :
      boost::lexical_cast<std::string>((int)width().toPixels()));
    element.setAttribute("height", height().isAuto() ? "" :
      boost::lexical_cast<std::string>((int)height().toPixels()));

    for(std::map<std::string, WString>::const_iterator i = parameters_.begin();
      i != parameters_.end(); ++i) {
        if (i->first != "flashvars") {
          DomElement *param = DomElement::createNew(DomElement_PARAM);
          param->setAttribute("name", i->first);
          param->setAttribute("value", i->second.toUTF8());
          obj->addChild(param);
        }
    }
    if (wApp->environment().agentIsIE()) {
      obj->setAttribute("classid", "clsid:D27CDB6E-AE6D-11cf-96B8-444553540000");
      // The next line is considered bad practice
      //obj->setAttribute("codebase",
      //"http://download.macromedia.com/pub/shockwave/cabs/flash/
      //swflash.cab#version=6,0,0,0");
      DomElement *param = DomElement::createNew(DomElement_PARAM);
      param->setAttribute("name", "movie");
      param->setAttribute("value", url_);
      obj->addChild(param);
    }
    if (variables_.size() > 0) {
      std::stringstream ss;
      for (std::map<std::string, WString>::const_iterator i = variables_.begin();
        i != variables_.end(); ++i) {
          if (i != variables_.begin())
            ss << "&";
          ss << Wt::Utils::urlEncode(i->first) << "="
            << Wt::Utils::urlEncode(i->second.toUTF8());
      }
      DomElement *param = DomElement::createNew(DomElement_PARAM);
      param->setAttribute("name", "flashvars");
      param->setAttribute("value", ss.str());
      obj->addChild(param);
    }
    if (alternative_)
      obj->addChild(alternative_->createSDomElement(wApp));
    element.addChild(obj);


  }
  WWebWidget::updateDom(element, all);
}

std::string WFlashObject::jsFlashRef() const
{
  return WT_CLASS ".getElement('" + id() + "_flash')";
}

void WFlashObject::getDomChanges(std::vector<DomElement *>& result,
                                 WApplication *app)
{
  WWebWidget::getDomChanges(result, app);
  if (sizeChanged_) {
    DomElement *element =
      DomElement::getForUpdate(id()  + "_flash", DomElement_OBJECT);
    element->setAttribute("width", width().isAuto() ? "" :
      boost::lexical_cast<std::string>((int)width().toPixels()));
    element->setAttribute("height", height().isAuto() ? "" :
      boost::lexical_cast<std::string>((int)height().toPixels()));
    result.push_back(element);
    sizeChanged_ = false;
  }
}

void WFlashObject::setAlternativeContent(WWidget *alternative)
{
  if (alternative_)
    delete alternative_;
  alternative_ = alternative;
  if (alternative_)
    addChild(alternative_);
}

void WFlashObject::resize(const WLength &width, const WLength &height)
{
  sizeChanged_ = true;
  WWebWidget::resize(width, height);
  repaint(Wt::RepaintPropertyAttribute);
}

DomElementType WFlashObject::domElementType() const
{
  return DomElement_DIV;
}

}
