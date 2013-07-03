// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010 Emweb bvba, Leuven, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WVideo"
#include "DomElement.h"

using namespace Wt;

WVideo::WVideo(WContainerWidget *parent):
  WAbstractMedia(parent),
  sizeChanged_(false),
  posterChanged_(false)
{
  setInline(false);
  this->setOptions(Controls);
}

void WVideo::updateMediaDom(DomElement& element, bool all)
{
  WAbstractMedia::updateMediaDom(element, all);
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
      element.setAttribute("poster", resolveRelativeUrl(posterUrl_));
    }
  }
  sizeChanged_ = posterChanged_ = false;
}

DomElement *WVideo::createMediaDomElement()
{
  return DomElement::createNew(DomElement_VIDEO);
}

std::string WVideo::jsVideoRef() const
{
  return jsMediaRef();
}

DomElementType WVideo::domElementType() const
{
  return DomElement_VIDEO;
}

void WVideo::setPoster(const std::string &url)
{
  posterUrl_ = url;
  posterChanged_ = true;
  repaint();
}

void WVideo::resize(const WLength &width, const WLength &height)
{
  sizeChanged_ = true;
  WWebWidget::resize(width, height);
}
