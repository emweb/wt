// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010 Emweb bvba, Leuven, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WHTML5Video"
#include "Wt/WApplication"
#include "Wt/WContainerWidget"
#include "Wt/WTemplate"
#include "DomElement.h"
#include "Utils.h"

using namespace Wt;

WHTML5Video::WHTML5Video(WContainerWidget *parent):
  WWebWidget(parent),
  preloadMode_(PreloadAuto),
  flags_(Controls),
  alternative_(0),
  sizeChanged_(false),
  posterChanged_(false),
  flagsChanged_(false),
  preloadChanged_(false)
{
  setInline(false);
}

void WHTML5Video::updateVideoDom(DomElement& element, bool all)
{
  if (all && alternative_) {
    element.setAttribute("onerror",
      """if(event.target.error.code=="
      ""   "event.target.error.MEDIA_ERR_SRC_NOT_SUPPORTED){"
      ""  "while (this.hasChildNodes())"
      ""    "if (" WT_CLASS ".hasTag(this.firstChild,'SOURCE')){"
      ""      "this.removeChild(this.firstChild);"
      ""    "}else{"
      ""      "this.parentNode.insertBefore(this.firstChild, this);"
      ""    "}"
      ""  "this.style.display= 'none';"
      """}"
      );
  }
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
      element.setAttribute("poster", posterUrl_);
    }
  }
  if (all || flagsChanged_) {
    if ((!all) || flags_ & Controls)
      element.setAttribute("controls", flags_ & Controls ? "controls" : "");
    if ((!all) || flags_ & Autoplay)
      element.setAttribute("autoplay", flags_ & Autoplay ? "autoplay" : "");
    if ((!all) || flags_ & Loop)
      element.setAttribute("loop", flags_ & Loop ? "loop" : "");
  }
  if (all || preloadChanged_) {
    switch (preloadMode_) {
      case PreloadNone:
        element.setAttribute("preload", "none");
        break;
      default:
      case PreloadAuto:
        element.setAttribute("preload", "auto");
        break;
      case PreloadMetadata:
        element.setAttribute("preload", "metadata");
        break;
    }
  }
  if (all) {
    for (std::size_t i = 0; i < sources_.size(); ++i) {
      DomElement *src = DomElement::createNew(DomElement_SOURCE);
      // src is mandatory
      src->setAttribute("src", sources_[i].url);
      if (sources_[i].hasType)
        src->setAttribute("type", sources_[i].type);
      if (sources_[i].hasMedia)
        src->setAttribute("media", sources_[i].media);
      if (i + 1 >= sources_.size() && alternative_) {
        // Last element -> add error handler for unsupported content
        src->setAttribute("onerror",
          """video = parentNode;"
          """if(video){"
          ""  "while (video && video.hasChildNodes())"
          ""    "if (" WT_CLASS ".hasTag(video.firstChild,'SOURCE')){"
          ""      "video.removeChild(video.firstChild);"
          ""    "}else{"
          ""      "video.parentNode.insertBefore(video.firstChild, video);"
          ""    "}"
          ""  "video.style.display= 'none';"
          """}"
          );
      }
      element.addChild(src);
    }
  }
  if (all)
    if (alternative_) {
      element.addChild(alternative_->createSDomElement(wApp));
    }
  sizeChanged_ = posterChanged_ = flagsChanged_ = preloadChanged_ = false;

}

DomElement *WHTML5Video::createDomElement(WApplication *app)
{
  DomElement *video = DomElement::createNew(DomElement_VIDEO);
  DomElement *wrap = 0;

  if (isInLayout()) {
    video->setProperty(PropertyStylePosition, "absolute");
    video->setProperty(PropertyStyleLeft, "0");
    video->setProperty(PropertyStyleRight, "0");
    wrap = DomElement::createNew(DomElement_DIV);
    wrap->setProperty(PropertyStylePosition, "relative");
    std::stringstream ss;
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

  DomElement *result = wrap ? wrap : video;
  setId(result, app);
  if (wrap) {
    videoId_ = id() + "_video";
    video->setId(videoId_);
  } else {
    videoId_ = id();
  }

  updateVideoDom(*video, true);

  if (wrap) {
    wrap->addChild(video);
  }
  updateDom(*result, true);

  return result;
}

void WHTML5Video::getDomChanges(std::vector<DomElement *>& result,
                                WApplication *app)
{
  DomElement *video = DomElement::getForUpdate(videoId_, DomElement_VIDEO);
  updateVideoDom(*video, false);
  result.push_back(video);
  WWebWidget::getDomChanges(result, app);
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

void WHTML5Video::setOptions(const WFlags<WHTML5Video::Options> &flags)
{
  flags_ = flags;
  flagsChanged_ = true;
  this->repaint(Wt::RepaintPropertyAttribute);
}

WFlags<WHTML5Video::Options> WHTML5Video::getOptions() const
{
  return flags_;
}

void WHTML5Video::setPreloadMode(PreloadMode mode)
{
  preloadMode_ = mode;
  preloadChanged_ = true;
  repaint(Wt::RepaintPropertyAttribute);
}

WHTML5Video::PreloadMode WHTML5Video::preloadMode() const
{
  return preloadMode_;
}

void WHTML5Video::addSource(const std::string &url)
{
  sources_.push_back(Source(url));
}

void WHTML5Video::addSource(const std::string &url, const std::string &type)
{
  sources_.push_back(Source(url, type));
}

void WHTML5Video::addSource(const std::string &url, const std::string &type,
                            const std::string &media)
{
  sources_.push_back(Source(url, type, media));
}

void WHTML5Video::setAlternativeContent(WWidget *alternative)
{
  if (alternative_)
    delete alternative_;
  alternative_ = alternative;
  if (alternative_)
    addChild(alternative_);
}

void WHTML5Video::resize(const WLength &width, const WLength &height)
{
  WWebWidget::resize(width, height);
  repaint(Wt::RepaintPropertyAttribute);
}
