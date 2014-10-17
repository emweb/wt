// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010 Emweb bvba, Leuven, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WAbstractMedia"
#include "Wt/WApplication"
#include "Wt/WEnvironment"
#include "Wt/WException"
#include "Wt/WResource"
#include "DomElement.h"
#include "WebUtils.h"

#ifndef WT_DEBUG_JS
#include "js/WAbstractMedia.min.js"
#endif

#include <boost/lexical_cast.hpp>

namespace {
  Wt::WAbstractMedia::ReadyState intToReadyState(int i) 
  {
    switch (i) {
    case 0:
      return Wt::WAbstractMedia::HaveNothing;
    case 1:
      return Wt::WAbstractMedia::HaveMetaData;
    case 2:
      return Wt::WAbstractMedia::HaveCurrentData;
    case 3:
      return Wt::WAbstractMedia::HaveFutureData;
    case 4:
      return Wt::WAbstractMedia::HaveEnoughData;
    default:
      throw Wt::WException("Invalid readystate");
    }
  }
}

using namespace Wt;
const char *WAbstractMedia::PLAYBACKSTARTED_SIGNAL = "play";
const char *WAbstractMedia::PLAYBACKPAUSED_SIGNAL = "pause";
const char *WAbstractMedia::ENDED_SIGNAL = "ended";
const char *WAbstractMedia::TIMEUPDATED_SIGNAL = "timeupdate";
const char *WAbstractMedia::VOLUMECHANGED_SIGNAL = "volumechange";

WAbstractMedia::WAbstractMedia(WContainerWidget *parent)
  : WInteractWidget(parent),
    sourcesRendered_(0),
    flags_(0),
    preloadMode_(PreloadAuto),
    alternative_(0),
    flagsChanged_(false),
    preloadChanged_(false),
    sourcesChanged_(false),
    playing_(false),
    volume_(-1),
    current_(-1),
    duration_(-1),
    ended_(false),
    readyState_(HaveNothing)
{
  setInline(false);
  setFormObject(true);

#ifndef WT_TARGET_JAVA
  implementStateless(&WAbstractMedia::play, &WAbstractMedia::play);
  implementStateless(&WAbstractMedia::pause, &WAbstractMedia::pause);
#endif //WT_TARGET_JAVA
}

WAbstractMedia::~WAbstractMedia()
{
  for (std::size_t i = 0; i < sources_.size(); ++i)
    delete sources_[i];
}

EventSignal<>& WAbstractMedia::playbackStarted()
{
  return *voidEventSignal(PLAYBACKSTARTED_SIGNAL, true);
}

EventSignal<>& WAbstractMedia::playbackPaused()
{
  return *voidEventSignal(PLAYBACKPAUSED_SIGNAL, true);
}

EventSignal<>& WAbstractMedia::ended()
{
  return *voidEventSignal(ENDED_SIGNAL, true);
}

EventSignal<>& WAbstractMedia::timeUpdated()
{
  return *voidEventSignal(TIMEUPDATED_SIGNAL, true);
}

EventSignal<>& WAbstractMedia::volumeChanged()
{
  return *voidEventSignal(VOLUMECHANGED_SIGNAL, true);
}

void WAbstractMedia::setFormData(const FormData& formData)
{
  if (!Utils::isEmpty(formData.values)) {
    std::vector<std::string> attributes;
    boost::split(attributes, formData.values[0], boost::is_any_of(";"));
    if (attributes.size() == 6) {
      try {
	volume_ = boost::lexical_cast<double>(attributes[0]);
      } catch (const std::exception& e) {
        volume_ = -1;
      }
      try {
	current_ = boost::lexical_cast<double>(attributes[1]);
      } catch (const std::exception& e) {
        current_ = -1;
      }
      try {
        duration_ = boost::lexical_cast<double>(attributes[2]);
      } catch (const std::exception& e) {
        duration_ = -1;
      }
      playing_ = (attributes[3] == "0");
      ended_ = (attributes[4] == "1");
      try {
        readyState_ = intToReadyState(boost::lexical_cast<int>(attributes[5]));
      } catch (const std::exception& e) {
	throw WException("WAbstractMedia: error parsing: " + formData.values[0]
			 + ": " + e.what());
      }
    } else
      throw WException("WAbstractMedia: error parsing: " + formData.values[0]);
  }
}

void WAbstractMedia::play()
{
  loadJavaScript();
  doJavaScript("jQuery.data(" + jsRef() + ", 'obj').play();");
}

void WAbstractMedia::pause()
{
  loadJavaScript();
  doJavaScript("jQuery.data(" + jsRef() + ", 'obj').pause();");
}

void WAbstractMedia::renderSource(DomElement* element,
				  WAbstractMedia::Source &source, bool isLast)
{
  // src is mandatory
  element->setAttribute("src", resolveRelativeUrl(source.link.url()));

  if (source.type != "")
    element->setAttribute("type", source.type);

  if (source.media != "")
    element->setAttribute("media", source.media);

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

void WAbstractMedia::updateMediaDom(DomElement& element, bool all)
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

void WAbstractMedia::loadJavaScript()
{
  if (javaScriptMember(" WAbstractMedia").empty()) {
    WApplication *app = WApplication::instance();

    LOAD_JAVASCRIPT(app, "js/WAbstractMedia.js", "WAbstractMedia", wtjs1);

    setJavaScriptMember(" WAbstractMedia",
			"new " WT_CLASS ".WAbstractMedia("
			+ app->javaScriptClass() + "," + jsRef() + ");");
  }
}

DomElement *WAbstractMedia::createDomElement(WApplication *app)
{
  loadJavaScript();

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
        ""  "if (v) {"
	""    "if (w >= 0) "
        ""      "v.setAttribute('width', w);"
        ""    "if (h >= 0) "
	""      "v.setAttribute('height', h);"
        ""  "}";
    }
    if (alternative_) {
      ss <<
        """a=" + alternative_->jsRef() + ";"
        ""  "if (a && a." << WT_RESIZE_JS <<")"
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

std::string WAbstractMedia::jsMediaRef() const
{
  if (mediaId_.empty()) {
    return "null";
  } else {
    return WT_CLASS ".getElement('" + mediaId_ + "')";
  }
}

void WAbstractMedia::getDomChanges(std::vector<DomElement *>& result,
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

void WAbstractMedia::setOptions(const WFlags<Options>& flags)
{
  flags_ = flags;
  flagsChanged_ = true;
  repaint();
}

WFlags<WAbstractMedia::Options> WAbstractMedia::getOptions() const
{
  return flags_;
}

void WAbstractMedia::setPreloadMode(PreloadMode mode)
{
  preloadMode_ = mode;
  preloadChanged_ = true;
  repaint();
}

WAbstractMedia::PreloadMode WAbstractMedia::preloadMode() const
{
  return preloadMode_;
}

void WAbstractMedia::clearSources()
{
  for (std::size_t i = 0; i < sources_.size(); ++i) {
    delete sources_[i];
  }
  sources_.clear();
  repaint();
}

void WAbstractMedia::addSource(const WLink& link, const std::string &type,
			       const std::string& media)
{
  sources_.push_back(new Source(this, link, type, media));
  sourcesChanged_ = true;
  repaint();
}

void WAbstractMedia::setAlternativeContent(WWidget *alternative)
{
  if (alternative_)
    delete alternative_;
  alternative_ = alternative;
  if (alternative_)
    addChild(alternative_);
}

WAbstractMedia::Source::Source(WAbstractMedia *parent,
			       const WLink& link, const std::string &type,
			       const std::string &media)
  :  parent(parent),
     type(type),
     media(media),
     link(link)
{
  if (link.type() == WLink::Resource)
    connection = link.resource()->dataChanged().connect
      (this, &Source::resourceChanged);
}

WAbstractMedia::Source::~Source()
{
  connection.disconnect();
}

void WAbstractMedia::Source::resourceChanged()
{
  parent->sourcesChanged_ = true;
  parent->repaint();
}
