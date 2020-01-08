// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WAbstractMedia.h"
#include "Wt/WApplication.h"
#include "Wt/WEnvironment.h"
#include "Wt/WException.h"
#include "Wt/WResource.h"
#include "DomElement.h"

#include "StringUtils.h"
#include "WebUtils.h"

#ifndef WT_DEBUG_JS
#include "js/WAbstractMedia.min.js"
#endif

namespace {
  Wt::MediaReadyState intToReadyState(int i) 
  {
    switch (i) {
    case 0:
      return Wt::MediaReadyState::HaveNothing;
    case 1:
      return Wt::MediaReadyState::HaveMetaData;
    case 2:
      return Wt::MediaReadyState::HaveCurrentData;
    case 3:
      return Wt::MediaReadyState::HaveFutureData;
    case 4:
      return Wt::MediaReadyState::HaveEnoughData;
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

WAbstractMedia::WAbstractMedia()
  : sourcesRendered_(0),
    flags_(None),
    preloadMode_(MediaPreloadMode::Auto),
    flagsChanged_(false),
    preloadChanged_(false),
    sourcesChanged_(false),
    playing_(false),
    volume_(-1),
    current_(-1),
    duration_(-1),
    ended_(false),
    readyState_(MediaReadyState::HaveNothing)
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
  manageWidget(alternative_, std::unique_ptr<WWidget>());
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
	volume_ = Utils::stod(attributes[0]);
      } catch (const std::exception& e) {
        volume_ = -1;
      }
      try {
	current_ = Utils::stod(attributes[1]);
      } catch (const std::exception& e) {
        current_ = -1;
      }
      try {
        duration_ = Utils::stod(attributes[2]);
      } catch (const std::exception& e) {
        duration_ = -1;
      }
      playing_ = (attributes[3] == "0");
      ended_ = (attributes[4] == "1");
      try {
        readyState_ = intToReadyState(Utils::stoi(attributes[5]));
      } catch (const std::exception& e) {
        readyState_ = MediaReadyState::HaveNothing;
      }
    } else
      throw WException("WAbstractMedia: error parsing: " 
		       + formData.values[0]);
  }
}

void WAbstractMedia::play()
{
  loadJavaScript();
  doJavaScript(jsRef() + ".wtObj.play();");
}

void WAbstractMedia::pause()
{
  loadJavaScript();
  doJavaScript(jsRef() + ".wtObj.pause();");
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
    if ((!all) || flags_.test(PlayerOption::Controls))
      element.setAttribute("controls",
			   flags_.test(PlayerOption::Controls) ? "controls" : "");
    if ((!all) || flags_.test(PlayerOption::Autoplay))
      element.setAttribute("autoplay",
			   flags_.test(PlayerOption::Autoplay) ? "autoplay" : "");
    if ((!all) || flags_.test(PlayerOption::Loop))
      element.setAttribute("loop",
			   flags_.test(PlayerOption::Loop) ? "loop" : "");
  }
  if (all || preloadChanged_) {
    switch (preloadMode_) {
    case MediaPreloadMode::None:
      element.setAttribute("preload", "none");
      break;
    default:
    case MediaPreloadMode::Auto:
      element.setAttribute("preload", "auto");
      break;
    case MediaPreloadMode::Metadata:
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

  DomElement *result = nullptr;

  if (isInLayout()) {
    // It's easier to set WT_RESIZE_JS after the following code,
    // but if it's not set, the alternative content will think that
    // it is not included in a layout manager. Set some phony function
    // now, which will be overwritten later in this method.
    setJavaScriptMember(WT_RESIZE_JS, "function() {}");
  }

  if (app->environment().agentIsIElt(9)) {
    // Shortcut: IE misbehaves when it encounters a media element
    result = DomElement::createNew(DomElementType::DIV);
    if (alternative_)
      result->addChild(alternative_->createSDomElement(app));
  } else {
    DomElement *media = createMediaDomElement();
    DomElement *wrap = nullptr;
    if (isInLayout()) {
      media->setProperty(Property::StylePosition, "absolute");
      media->setProperty(Property::StyleLeft, "0");
      media->setProperty(Property::StyleRight, "0");
      wrap = DomElement::createNew(DomElementType::DIV);
      wrap->setProperty(Property::StylePosition, "relative");
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
      DomElement *src = DomElement::createNew(DomElementType::SOURCE);
      src->setId(mediaId_ + "s" + std::to_string(i));
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
    DomElement *media = DomElement::getForUpdate(mediaId_, DomElementType::DIV);
    updateMediaDom(*media, false);
    if (sourcesChanged_) {
      // Updating source elements seems to be ill-supported in at least FF,
      // so we delete them all and reinsert them.
      // Delete source elements that are no longer required
      for (std::size_t i = 0; i < sourcesRendered_; ++i)
	media->callJavaScript
	  (WT_CLASS ".remove('" + mediaId_ + "s" + std::to_string(i) + "');",
	   true);
      sourcesRendered_ = 0;
      for (std::size_t i = 0; i < sources_.size(); ++i) {
        DomElement *src = DomElement::createNew(DomElementType::SOURCE);
        src->setId(mediaId_ + "s" + std::to_string(i));
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

void WAbstractMedia::setOptions(const WFlags<PlayerOption>& flags)
{
  flags_ = flags;
  flagsChanged_ = true;
  repaint();
}

WFlags<PlayerOption> WAbstractMedia::getOptions() const
{
  return flags_;
}

void WAbstractMedia::setPreloadMode(MediaPreloadMode mode)
{
  preloadMode_ = mode;
  preloadChanged_ = true;
  repaint();
}

MediaPreloadMode WAbstractMedia::preloadMode() const
{
  return preloadMode_;
}

void WAbstractMedia::clearSources()
{
  sources_.clear();
  repaint();
}

void WAbstractMedia::addSource(const WLink& link, const std::string &type,
			       const std::string& media)
{
  sources_.push_back
    (std::unique_ptr<Source>(new Source(this, link, type, media)));
  sourcesChanged_ = true;
  repaint();
}

void WAbstractMedia::setAlternativeContent(std::unique_ptr<WWidget> alternative)
{
  manageWidget(alternative_, std::move(alternative));
}

void WAbstractMedia::iterateChildren(const HandleWidgetMethod& method) const
{
  if (alternative_)
#ifndef WT_TARGET_JAVA
    method(alternative_.get());
#else
    method.handle(alternative_.get());
#endif
}

void WAbstractMedia::enableAjax() 
{
  WWebWidget::enableAjax();

  if (flags_.test(PlayerOption::Autoplay)) {
    // chrome stops playing as soon as the widget tree is changed
    // We therefore restart the play manually
    play();
  }
}	

WAbstractMedia::Source::Source(WAbstractMedia *parent,
			       const WLink& link, const std::string &type,
			       const std::string &media)
  :  parent(parent),
     type(type),
     media(media),
     link(link)
{
  if (link.type() == LinkType::Resource) {
    /*
    connection = link.resource()->dataChanged().connect
      ([=]() { this->resourceChanged(); });
    */
#ifdef WT_TARGET_JAVA
    connection = link.resource()->dataChanged().connect
      (this, std::bind(&Source::resourceChanged, this));
#else // !WT_TARGET_JAVA
    connection = link.resource()->dataChanged().connect
      (std::bind(&Source::resourceChanged, this));
#endif // WT_TARGET_JAVA
  }
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

