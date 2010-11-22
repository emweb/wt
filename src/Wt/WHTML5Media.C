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
#include "Wt/WResource"
#include "Wt/WLogger"
#include "DomElement.h"
#include "Utils.h"

#include "JavaScriptLoader.h"
#ifndef WT_DEBUG_JS
#include "js/WHTML5Media.min.js"
#endif

#include <boost/algorithm/string.hpp>

using namespace Wt;
const char *WHTML5Media::PLAYBACKSTARTED_SIGNAL = "play";
const char *WHTML5Media::PLAYBACKPAUSED_SIGNAL = "pause";
const char *WHTML5Media::ENDED_SIGNAL = "ended";
const char *WHTML5Media::TIMEUPDATED_SIGNAL = "timeupdate";
const char *WHTML5Media::VOLUMECHANGED_SIGNAL = "volumechange";

WHTML5Media::WHTML5Media(WContainerWidget *parent):
  WInteractWidget(parent),
  sourcesRendered_(0),
  flags_(0),
  preloadMode_(PreloadAuto),
  alternative_(0),
  flagsChanged_(false),
  preloadChanged_(false),
  sourcesChanged_(false),
  playing_(false)
{
  setInline(false);
  setFormObject(true);

  WApplication *app = wApp;
  const char *THIS_JS = "js/WHTML5Media.js";
  if (!app->javaScriptLoaded(THIS_JS)) {
    LOAD_JAVASCRIPT(app, THIS_JS, "WHTML5Media", wtjs1);
    app->setJavaScriptLoaded(THIS_JS);
  }
  doJavaScript("new " WT_CLASS ".WHTML5Media("
    + app->javaScriptClass() + "," + jsRef() + ");");
  setJavaScriptMember("WtPlay",
    "function() {jQuery.data(" + jsRef() + ", 'obj').play();}");
  setJavaScriptMember("WtPause",
    "function() {jQuery.data(" + jsRef() + ", 'obj').pause();}");

#ifndef WT_TARGET_JAVA
  implementStateless(&WHTML5Media::play, &WHTML5Media::play);
  implementStateless(&WHTML5Media::pause, &WHTML5Media::pause);
#endif //WT_TARGET_JAVA
}

WHTML5Media::~WHTML5Media()
{
  for (std::size_t i = 0; i < sources_.size(); ++i)
    delete sources_[i];
}

EventSignal<>& WHTML5Media::playbackStarted()
{
  return *voidEventSignal(PLAYBACKSTARTED_SIGNAL, true);
}

EventSignal<>& WHTML5Media::playbackPaused()
{
  return *voidEventSignal(PLAYBACKPAUSED_SIGNAL, true);
}

EventSignal<>& WHTML5Media::ended()
{
  return *voidEventSignal(ENDED_SIGNAL, true);
}

EventSignal<>& WHTML5Media::timeUpdated()
{
  return *voidEventSignal(TIMEUPDATED_SIGNAL, true);
}

EventSignal<>& WHTML5Media::volumeChanged()
{
  return *voidEventSignal(VOLUMECHANGED_SIGNAL, true);
}

void WHTML5Media::setFormData(const FormData& formData)
{
  if (!Utils::isEmpty(formData.values)) {
    std::vector<std::string> attributes;
    boost::split(attributes, formData.values[0], boost::is_any_of(";"));
    if (attributes.size() == 5) {
      double volume, current, duration;
      bool paused, ended;
      try {
        volume = boost::lexical_cast<double>(attributes[0]);
        current = boost::lexical_cast<double>(attributes[1]);
        duration = boost::lexical_cast<double>(attributes[2]);
        paused = (attributes[3] == "1");
        ended = (attributes[4] == "1");

	playing_ = !paused;

	// Are other values any useful ?
      } catch (boost::bad_lexical_cast &e) {
	WApplication::instance()->log("error")
	  << "WHTML5Media: could not parse form data: " << formData.values[0];
      }
    }
  }
}

void WHTML5Media::play()
{
  doJavaScript(jsRef() + ".WtPlay();");
}

void WHTML5Media::pause()
{
  doJavaScript(jsRef() + ".WtPause();");
}

void WHTML5Media::renderSource(DomElement* element,
                               WHTML5Media::Source &source, bool isLast)
{
  // src is mandatory
  element->setAttribute("src", fixRelativeUrl(source.url));
  if (source.type != "") element->setAttribute("type", source.type);
  if (source.media != "") element->setAttribute("media", source.media);
  if (isLast && alternative_) {
    // Last element -> add error handler for unsupported content
    element->setAttribute("onerror",
      """var media = this.parentNode;"
      """if(media){"
      ""  "while (media && media.children.length)"
      ""    "if (" WT_CLASS ".hasTag(media.firstChild,'SOURCE')){"
      ""      "media.removeChild(media.firstChild);"
      ""    "}else{"
      ""      "media.parentNode.insertBefore(media.firstChild, media);"
      ""    "}"
      ""  "media.style.display= 'none';"
      """}"
      );
  } else {
    element->setAttribute("onerror", "");
  }
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

  updateEventSignals(element, all);

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

  if (app->environment().agentIsIElt(9) ||
      app->environment().agent() == WEnvironment::MobileWebKitAndroid) {
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
    // Create the 'source' elements
    for (std::size_t i = 0; i < sources_.size(); ++i) {
      DomElement *src = DomElement::createNew(DomElement_SOURCE);
      src->setId(mediaId_ + "s" + boost::lexical_cast<std::string>(i));
      renderSource(src, *sources_[i], i + 1 >= sources_.size());
      media->addChild(src);
    }
    sourcesRendered_ = sources_.size();
    sourcesChanged_ = false;

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

  if (isInLayout()) {
    result->setEvent(PLAYBACKSTARTED_SIGNAL, std::string());
    result->setEvent(PLAYBACKPAUSED_SIGNAL, std::string());
    result->setEvent(ENDED_SIGNAL, std::string());
    result->setEvent(TIMEUPDATED_SIGNAL, std::string());
    result->setEvent(VOLUMECHANGED_SIGNAL, std::string());
  }

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
    if (sourcesChanged_) {
      // Updating source elements seems to be ill-supported in at least FF,
      // so we delete them all and reinsert them.
      // Delete source elements that are no longer required
      for (std::size_t i = 0; i < sourcesRendered_; ++i)
	media->callJavaScript
	  (WT_CLASS ".remove('" + mediaId_ + "s"
	   + boost::lexical_cast<std::string>(i) + "');",
	   true);
      sourcesRendered_ = 0;
      for (std::size_t i = 0; i < sources_.size(); ++i) {
        DomElement *src = DomElement::createNew(DomElement_SOURCE);
        src->setId(mediaId_ + "s" + boost::lexical_cast<std::string>(i));
        renderSource(src, *sources_[i], i + 1 >= sources_.size());
        media->addChild(src);
      }
      sourcesRendered_ = sources_.size();
      sourcesChanged_ = false;
      // Explicitly request rerun of media selection algorithm
      // 4.8.9.2 says it should happen automatically, but FF doesn't
      media->callJavaScript(jsMediaRef() + ".load();");
    }
    result.push_back(media);
  }
  WInteractWidget::getDomChanges(result, app);
}

void WHTML5Media::setOptions(const WFlags<Options> &flags)
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

void WHTML5Media::clearSources()
{
  for (std::size_t i = 0; i < sources_.size(); ++i) {
    delete sources_[i];
  }
  sources_.clear();
  repaint(Wt::RepaintPropertyAttribute);
}

void WHTML5Media::addSource(const std::string &url, const std::string &type,
                            const std::string &media)
{
  sources_.push_back(new Source(url, type, media));
  sourcesChanged_ = true;
  repaint(Wt::RepaintPropertyAttribute);
}

void WHTML5Media::addSource(WResource *resource,
                            const std::string &type,
                            const std::string &media)
{
  sources_.push_back(new Source(this, resource, type, media));
  sourcesChanged_ = true;
  repaint(Wt::RepaintPropertyAttribute);
}

void WHTML5Media::setAlternativeContent(WWidget *alternative)
{
  if (alternative_)
    delete alternative_;
  alternative_ = alternative;
  if (alternative_)
    addChild(alternative_);
}

WHTML5Media::Source::Source(WHTML5Media *parent,
                            WResource *resource, const std::string &type,
                            const std::string &media)
  :  parent(parent),
     type(type),
     url(resource->url()),
     media(media),
     resource(resource)
{
  connection = resource->dataChanged().connect(this, &Source::resourceChanged);
}

WHTML5Media::Source::Source(const std::string &url, const std::string &type,
			    const std::string &media)
  : type(type), url(url), media(media)
{ }

WHTML5Media::Source::~Source()
{
  connection.disconnect();
}

void WHTML5Media::Source::resourceChanged()
{
  url = resource->url();
  parent->sourcesChanged_ = true;
  parent->repaint(Wt::RepaintPropertyAttribute);
}
