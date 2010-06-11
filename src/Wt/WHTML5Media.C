// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010 Emweb bvba, Leuven, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WHTML5Media"
#include "Wt/WApplication"
#include "Wt/WContainerWidget"
#include "Wt/WEnvironment"
#include "DomElement.h"
#include "Utils.h"

#include "JavaScriptLoader.h"
#ifndef WT_DEBUG_JS
#include "js/WHTML5Media.min.js"
#endif

using namespace Wt;

WHTML5Media::WHTML5Media(WContainerWidget *parent):
  WWebWidget(parent),
  flags_(0),
  preloadMode_(PreloadAuto),
  alternative_(0),
  flagsChanged_(false),
  preloadChanged_(false)
{
  setInline(false);

  WApplication *app = wApp;
  const char *THIS_JS = "js/WHTML5Media.js";
  if (!app->javaScriptLoaded(THIS_JS)) {
    LOAD_JAVASCRIPT(app, THIS_JS, "WHTML5Media", wtjs1);
    app->setJavaScriptLoaded(THIS_JS);
  }
  doJavaScript("new " WT_CLASS ".WHTML5Media("
    + app->javaScriptClass() + "," + jsRef() + ");");
  setJavaScriptMember("WtPlay",
    "function() {$('#" + id() + "').data('obj').play();}");
  setJavaScriptMember("WtPause",
    "function() {$('#" + id() + "').data('obj').pause();}");

#ifndef WT_TARGET_JAVA
  implementStateless(&WHTML5Media::play, &WHTML5Media::play);
  implementStateless(&WHTML5Media::pause, &WHTML5Media::pause);
#endif //WT_TARGET_JAVA
}

void WHTML5Media::play()
{
  doJavaScript(jsRef() + ".WtPlay();");
}

void WHTML5Media::pause()
{
  doJavaScript(jsRef() + ".WtPause();");
}

void WHTML5Media::updateMediaDom(DomElement& element, bool all)
{
  // Only if not IE
  if (all && alternative_) {
    element.setAttribute("onerror",
      """if(event.target.error && event.target.error.code=="
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
      src->setAttribute("src", fixRelativeUrl(sources_[i].url));
      if (sources_[i].hasType)
        src->setAttribute("type", sources_[i].type);
      if (sources_[i].hasMedia)
        src->setAttribute("media", sources_[i].media);
      if (i + 1 >= sources_.size() && alternative_) {
        // Last element -> add error handler for unsupported content
        src->setAttribute("onerror",
          """media = parentNode;"
          """if(media){"
          ""  "while (media && media.hasChildNodes())"
          ""    "if (" WT_CLASS ".hasTag(media.firstChild,'SOURCE')){"
          ""      "media.removeChild(media.firstChild);"
          ""    "}else{"
          ""      "media.parentNode.insertBefore(media.firstChild, media);"
          ""    "}"
          ""  "media.style.display= 'none';"
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
  flagsChanged_ = preloadChanged_ = false;
}

DomElement *WHTML5Media::createDomElement(WApplication *app)
{
  DomElement *result = 0;

  if (isInLayout()) {
    // It's easier to set WT_RESIZE_JS after the following code,
    // but if it's not set, the alternative content will think that
    // it is not included in a layout manager. Set some phony function
    // now, which will be overwritten later in this method.
    setJavaScriptMember(WT_RESIZE_JS, "function() {}");
  }

  if (app->environment().agentIsIE()) {
    // Shortcut: IE misbehaves when it encounters a media element
    result = DomElement::createNew(DomElement_DIV);
    if (alternative_)
      result->addChild(alternative_->createSDomElement(app));
  } else {
    DomElement *media = createMediaDomElement();
    DomElement *wrap = 0;
    if (isInLayout()) {
      media->setProperty(PropertyStylePosition, "absolute");
      media->setProperty(PropertyStyleLeft, "0");
      media->setProperty(PropertyStyleRight, "0");
      wrap = DomElement::createNew(DomElement_DIV);
      wrap->setProperty(PropertyStylePosition, "relative");
    }
    result = wrap ? wrap : media;
    if (wrap) {
      mediaId_ = id() + "_media";
      media->setId(mediaId_);
    } else {
      mediaId_ = id();
    }

    updateMediaDom(*media, true);


    if (wrap) {
      wrap->addChild(media);
    }
  }

  if (isInLayout()) {
    std::stringstream ss;
    ss <<
      """function(self, w, h) {";
    if (!mediaId_.empty()) {
      ss <<
        ""  "v=" + jsMediaRef() + ";"
        ""  "if(v){"
        ""    "v.setAttribute('width', w);"
        ""    "v.setAttribute('height', h);"
        ""  "}";
    }
    if (alternative_) {
      ss <<
        """a=" + alternative_->jsRef() + ";"
        ""  "if(a && a." << WT_RESIZE_JS <<")"
        ""    "a." << WT_RESIZE_JS << "(a, w, h);";
    }
    ss
      <<"}";
    setJavaScriptMember(WT_RESIZE_JS, ss.str());
  }

  setId(result, app);
  updateDom(*result, true);
  setJavaScriptMember("mediaId", "'" + mediaId_ + "'");

  return result;
}

std::string WHTML5Media::jsMediaRef() const
{
  if (mediaId_.empty()) {
    return "null";
  } else {
    return WT_CLASS ".getElement('" + mediaId_ + "')";
  }
}

void WHTML5Media::getDomChanges(std::vector<DomElement *>& result,
                                WApplication *app)
{
  if (!mediaId_.empty()) {
    DomElement *media = DomElement::getForUpdate(mediaId_, DomElement_DIV);
    updateMediaDom(*media, false);
    result.push_back(media);
  }
  WWebWidget::getDomChanges(result, app);
}

void WHTML5Media::setOptions(const WFlags<WHTML5Media::Options> &flags)
{
  flags_ = flags;
  flagsChanged_ = true;
  this->repaint(Wt::RepaintPropertyAttribute);
}

WFlags<WHTML5Media::Options> WHTML5Media::getOptions() const
{
  return flags_;
}

void WHTML5Media::setPreloadMode(PreloadMode mode)
{
  preloadMode_ = mode;
  preloadChanged_ = true;
  repaint(Wt::RepaintPropertyAttribute);
}

WHTML5Media::PreloadMode WHTML5Media::preloadMode() const
{
  return preloadMode_;
}

void WHTML5Media::addSource(const std::string &url)
{
  sources_.push_back(Source(url));
}

void WHTML5Media::addSource(const std::string &url, const std::string &type)
{
  sources_.push_back(Source(url, type));
}

void WHTML5Media::addSource(const std::string &url, const std::string &type,
                            const std::string &media)
{
  sources_.push_back(Source(url, type, media));
}

void WHTML5Media::setAlternativeContent(WWidget *alternative)
{
  if (alternative_)
    delete alternative_;
  alternative_ = alternative;
  if (alternative_)
    addChild(alternative_);
}

