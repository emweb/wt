// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010 Emweb bvba, Leuven, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WHTML5Video"
#include "Wt/WApplication"
#include "Wt/WContainerWidget"
#include "Wt/WEnvironment"
#include "Wt/WTemplate"
#include "DomElement.h"
#include "Utils.h"

using namespace Wt;

WHTML5Video::WHTML5Video(WContainerWidget *parent):
  WHTML5Media(parent),
  sizeChanged_(false),
  posterChanged_(false)
{
  setInline(false);
  this->setOptions(Controls);
}

void WHTML5Video::updateMediaDom(DomElement& element, bool all)
{
  WHTML5Media::updateMediaDom(element, all);
  // Video has a few extra attributes...
  if (all || sizeChanged_) {
    if ((!all) || !width().isAuto())
      element.setAttribute("width",
        width().isAuto() ? "" :
          boost::lexical_cast<std::string>((int)width().toPixels()));
    if ((!all) || !height().isAuto())
      element.setAttribute("height",
        height().isAuto() ? "" :
          boost::lexical_cast<std::string>((int)height().toPixels()));
  }
  if (all || posterChanged_) {
    if ((!all) || posterUrl_ != "") {
      element.setAttribute("poster", fixRelativeUrl(posterUrl_));
    }
  }
  sizeChanged_ = posterChanged_ = false;
}

DomElement *WHTML5Video::createMediaDomElement()
{
  return DomElement::createNew(DomElement_VIDEO);
}

std::string WHTML5Video::jsVideoRef() const
{
  return jsMediaRef();
}

DomElementType WHTML5Video::domElementType() const
{
  return DomElement_VIDEO;
}

void WHTML5Video::setPoster(const std::string &url)
{
  posterUrl_ = url;
  posterChanged_ = true;
  this->repaint(Wt::RepaintPropertyAttribute);
}

void WHTML5Video::resize(const WLength &width, const WLength &height)
{
  sizeChanged_ = true;
  WWebWidget::resize(width, height);
  repaint(Wt::RepaintPropertyAttribute);
}
