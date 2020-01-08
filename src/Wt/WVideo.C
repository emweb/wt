// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WVideo.h"
#include "DomElement.h"

using namespace Wt;

WVideo::WVideo()
  : sizeChanged_(false),
    posterChanged_(false)
{
  setInline(false);
  this->setOptions(PlayerOption::Controls);
}

void WVideo::updateMediaDom(DomElement& element, bool all)
{
  WAbstractMedia::updateMediaDom(element, all);
  // Video has a few extra attributes...
  if (all || sizeChanged_) {
    if ((!all) || !width().isAuto())
      element.setAttribute("width",
        width().isAuto() ? "" : std::to_string((int)width().toPixels()));
    if ((!all) || !height().isAuto())
      element.setAttribute("height",
        height().isAuto() ? "" : std::to_string((int)height().toPixels()));
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
  return DomElement::createNew(DomElementType::VIDEO);
}

std::string WVideo::jsVideoRef() const
{
  return jsMediaRef();
}

DomElementType WVideo::domElementType() const
{
  return DomElementType::VIDEO;
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
