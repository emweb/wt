// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2011 Emweb bvba, Leuven, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WMediaPlayer"
#include "Wt/WAnchor"
#include "Wt/WApplication"
#include "Wt/WEnvironment"
#include "Wt/WException"
#include "Wt/WProgressBar"
#include "Wt/WStringStream"
#include "Wt/WTemplate"
#include "Wt/WText"
#include "WebUtils.h"

#ifndef WT_DEBUG_JS
#include "js/WMediaPlayer.min.js"
#endif

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

namespace {
  Wt::WMediaPlayer::ReadyState intToReadyState(int i) 
  {
    switch (i) {
    case 0:
      return Wt::WMediaPlayer::HaveNothing;
    case 1:
      return Wt::WMediaPlayer::HaveMetaData;
    case 2:
      return Wt::WMediaPlayer::HaveCurrentData;
    case 3:
      return Wt::WMediaPlayer::HaveFutureData;
    case 4:
      return Wt::WMediaPlayer::HaveEnoughData;
    default:
      throw Wt::WException("Invalid readystate");
    }
  }
}

namespace Wt {

const char *WMediaPlayer::LOAD_STARTED_SIGNAL = "jPlayer_loadstart.Wt";
const char *WMediaPlayer::TIME_UPDATED_SIGNAL = "jPlayer_timeupdate.Wt";
const char *WMediaPlayer::PLAYBACK_STARTED_SIGNAL = "jPlayer_play.Wt";
const char *WMediaPlayer::PLAYBACK_PAUSED_SIGNAL = "jPlayer_pause.Wt";
const char *WMediaPlayer::ENDED_SIGNAL = "jPlayer_ended.Wt";
const char *WMediaPlayer::VOLUME_CHANGED_SIGNAL = "jPlayer_volumechange.Wt";

class WMediaPlayerImpl : public WTemplate
{
public:
  WMediaPlayerImpl(WMediaPlayer *player, const WString& text)
    : WTemplate(text),
      player_(player)
  { 
    setFormObject(true);
  }

protected:
  virtual std::string renderRemoveJs()
  {
    if (isRendered())
      return player_->jsPlayerRef() + ".jPlayer('destroy');"
	WT_CLASS ".remove('" + id() + "');";
    else
      return WTemplate::renderRemoveJs();
  }

  virtual void setFormData(const FormData& formData)
  {
    player_->setFormData(formData);
  }

private:
  WMediaPlayer *player_;
};

/*
 * TODO:
 * - setters -> modify status_
 * - IE: keeps sending signals when ended ?
 */
WMediaPlayer::State::State()
  : playing(false),
    readyState(HaveNothing),
    seekPercent(0),
    volume(0.8),
    duration(0),
    currentTime(0),
    playbackRate(1)
{ }

WMediaPlayer::WMediaPlayer(MediaType mediaType, WContainerWidget *parent)
  : WCompositeWidget(parent),
    mediaType_(mediaType),
    videoWidth_(0),
    videoHeight_(0),
    gui_(this),
    boundSignals_(0)
{
  for (unsigned i = 0; i < 11; ++i)
    control_[i] = 0;

  for (unsigned i = 0; i < 3; ++i)
    display_[i] = 0;

  for (unsigned i = 0; i < 2; ++i)
    progressBar_[i] = 0;

  WTemplate *impl = new WMediaPlayerImpl(this, tr("Wt.WMediaPlayer.template"));
  impl->bindString("gui", std::string());

  setImplementation(impl);

  WApplication *app = WApplication::instance();

  LOAD_JAVASCRIPT(app, "js/WMediaPlayer.js", "WMediaPlayer", wtjs1);

  std::string res = WApplication::relativeResourcesUrl() + "jPlayer/";

  if (!app->environment().ajax())
    app->require(res + "jquery.min.js");

  if (app->require(res + "jquery.jplayer.min.js"))
    app->useStyleSheet(res + "skin/jplayer.blue.monday.css");

  if (mediaType_ == Video)
    setVideoSize(480, 270);

#ifndef WT_TARGET_JAVA
  implementJavaScript(&WMediaPlayer::play,
		      jsPlayerRef() + ".jPlayer('play');");
  implementJavaScript(&WMediaPlayer::pause,
		      jsPlayerRef() + ".jPlayer('pause');");
  implementJavaScript(&WMediaPlayer::stop,
		      jsPlayerRef() + ".jPlayer('stop');");
#endif
}

WMediaPlayer::~WMediaPlayer()
{
  setParentWidget(0); // to have virtual renderRemoveJs()

  for (unsigned i = 0; i < signals_.size(); ++i)
    delete signals_[i];
}

void WMediaPlayer::clearSources()
{
  media_.clear();

  mediaUpdated_ = true;
  scheduleRender();
}

void WMediaPlayer::addSource(Encoding encoding, const WLink& link)
{
  media_.push_back(Source());
  media_.back().link = link;
  media_.back().encoding = encoding;

  mediaUpdated_ = true;
  scheduleRender();
}

WLink WMediaPlayer::getSource(Encoding encoding) const
{
  for (unsigned i = 0; i < media_.size(); ++i) {
    if (media_[i].encoding == encoding)
      return media_[i].link;
  }

  return WLink("");
}

void WMediaPlayer::setTitle(const WString& title)
{
  title_ = title;

  if (display_[Title]) {
    display_[Title]->setText(title_);

    if (gui_) {
      WTemplate *t = dynamic_cast<WTemplate *>(gui_);
      if (t)
	t->bindString("title-display", title_.empty() ? "none" : "");
    }
  }
}

void WMediaPlayer::setControlsWidget(WWidget *controlsWidget)
{
  if (gui_ != this)
    delete gui_;

  gui_ = controlsWidget;

  WTemplate *impl = dynamic_cast<WTemplate *>(implementation());

  if (gui_) {
    gui_->addStyleClass("jp-gui");

    impl->bindWidget("gui", gui_);
  } else
    impl->bindString("gui", std::string());
}

WWidget *WMediaPlayer::controlsWidget() const
{
  if (gui_ == this)
    (const_cast<WMediaPlayer *>(this))->createDefaultGui();

  return gui_;
}

void WMediaPlayer::setButton(ButtonControlId id, WInteractWidget *w)
{
  delete control_[id];
  control_[id] = w;
}

WInteractWidget *WMediaPlayer::button(ButtonControlId id) const
{
  controlsWidget(); // may lazy-create the default gui.

  return control_[id];
}

void WMediaPlayer::setText(TextId id, WText *w)
{
  delete display_[id];
  display_[id] = w;

  if (id == Title && w)
    w->setText(title_);
}

WText *WMediaPlayer::text(TextId id) const
{
  controlsWidget(); // may lazy-create the default gui.

  return display_[id];
}

void WMediaPlayer::setProgressBar(BarControlId id, WProgressBar *w)
{
  const BarControlId bc_id = id;

  delete progressBar_[id];
  progressBar_[id] = w;

  if (w) {
    w->setFormat(WString::Empty);

    w->valueChanged().connect
      (boost::bind(&WMediaPlayer::updateFromProgressBar, this, bc_id, _1));

    updateProgressBarState(id);
  }
}

void WMediaPlayer::updateProgressBarState(BarControlId id)
{
  WProgressBar *bar = progressBar(id);
  if (bar) {
    switch (id) {
    case Time:
      bar->setState(0, status_.seekPercent * status_.duration,
		    status_.currentTime);
      break;
    case Volume:
      bar->setState(0, 1, status_.volume);
    }
  }
}

void WMediaPlayer::updateFromProgressBar(BarControlId id, double value)
{
  switch (id) {
  case Time:
    seek(value); break;
  case Volume:
    setVolume(value);
  } 
}

WProgressBar *WMediaPlayer::progressBar(BarControlId id) const
{
  controlsWidget(); // may lazy-create the default gui.

  return progressBar_[id];
}

std::string WMediaPlayer::jsPlayerRef() const
{
  return "$('#" + id() + " .jp-jplayer')";
}

void WMediaPlayer::play()
{
  if (isRendered()) {
    /*
     * play is being delayed so that other changes (e.g. addSource() are
     * reflected first, see #2819
     */
    doJavaScript("setTimeout(function(){" + jsPlayerRef() 
		 + ".jPlayer('play'); }, 0);");
  } else
    playerDo("play");
}

void WMediaPlayer::pause()
{
  playerDo("pause");
}

void WMediaPlayer::setVolume(double volume)
{
  status_.volume = volume;
  playerDo("volume", boost::lexical_cast<std::string>(volume));
}

double WMediaPlayer::volume() const
{
  return status_.volume;
}

void WMediaPlayer::setPlaybackRate(double rate)
{
  if (rate != status_.playbackRate) {
    status_.playbackRate = rate;

    playerDoData("wtPlaybackRate", boost::lexical_cast<std::string>(rate));
  }
}

void WMediaPlayer::mute(bool mute)
{
  playerDo(mute ? "mute" : "unmute");
}

void WMediaPlayer::stop()
{
  playerDo("stop");
}

void WMediaPlayer::seek(double time)
{
  if (status_.seekPercent != 0) {
    double pct = time / (status_.seekPercent * status_.duration / 100);
    pct = std::min(1.0, pct);

    playerDo("playHead", boost::lexical_cast<std::string>(pct * 100));
  }
}

void WMediaPlayer::setVideoSize(int width, int height)
{
  if (width != videoWidth_ || height != videoHeight_) {
    videoWidth_ = width;
    videoHeight_ = height;

    setWidth(videoWidth_);

    if (isRendered()) {
      WStringStream ss;

      ss << "'size', {"
	 <<   "width: \"" << videoWidth_ << "px\","
	 <<   "height: \"" << videoHeight_ << "px\","
	 <<   "cssClass: \"jp-video-" << videoHeight_ << "p\""
	 << "}";

      playerDo("option", ss.str());
    }
  }
}

void WMediaPlayer::playerDoData(const std::string& method,
				const std::string& args)
{
  playerDoRaw(".data('jPlayer')." + method + "(" + args + ")");
}

void WMediaPlayer::playerDo(const std::string& method, const std::string& args)
{
  WStringStream ss;

  ss << ".jPlayer('" << method << '\'';

  if (!args.empty()) {
    ss << ',' << args;
  }

  ss << ')';

  playerDoRaw(ss.str());
}

void WMediaPlayer::playerDoRaw(const std::string& jqueryMethod)
{
  WStringStream ss;

  if (isRendered())
    ss << jsPlayerRef();

  ss << jqueryMethod;

  if (isRendered())
    ss << ';';

  if (isRendered())
    doJavaScript(ss.str());
  else 
    initialJs_ += ss.str();
}


void WMediaPlayer::refresh()
{
  WCompositeWidget::refresh();

  // rerender so that the jPlayer constructor is executed on the new HTML
  render(RenderFull);
}

void WMediaPlayer::render(WFlags<RenderFlag> flags)
{
  // XXX subtitles, chapters, stream ?
  static const char *mediaNames[] = {
    "poster",
    "mp3", "m4a", "oga", "wav", "webma", "fla",
    "m4v", "ogv", "webmv", "flv"
  };

  WApplication *app = WApplication::instance();

  if (mediaUpdated_) {
    WStringStream ss;

    ss << '{';

    bool first = true;
    for (unsigned i = 0; i < media_.size(); ++i) {
      if (media_[i].link.isNull())
	continue;

      if (!first)
	ss << ',';

      std::string url = app->resolveRelativeUrl(media_[i].link.url());

      ss << const_cast<char *>(mediaNames[media_[i].encoding]) << ": "
	 << WWebWidget::jsStringLiteral(url);

      first = false;
    }

    ss << '}';

    if (!(flags & RenderFull))
      playerDo("setMedia", ss.str());
    else {
      initialJs_ = ".jPlayer('setMedia', " + ss.str() + ')' + initialJs_;
    }

    mediaUpdated_ = false;
  }

  if (flags & RenderFull) {
    if (gui_ == this)
      createDefaultGui();

    WStringStream ss;

    ss << jsPlayerRef() << ".jPlayer({"
       << "ready: function () {";

    if (!initialJs_.empty())
      ss << "$(this)" << initialJs_ << ';';

    initialJs_.clear();

    ss << "},"
       << "swfPath: \"" << WApplication::resourcesUrl() << "jPlayer\","
       << "supplied: \"";

    bool first = true;
    for (unsigned i = 0; i < media_.size(); ++i) {
      if (media_[i].encoding != PosterImage) {
	if (!first)
	  ss << ',';
	ss << const_cast<char *>(mediaNames[media_[i].encoding]);
	first = false;
      }
    }

    ss << "\",";

    if (mediaType_ == Video) {
      ss << "size: {"
	 <<   "width: \"" << videoWidth_ << "px\","
	 <<   "height: \"" << videoHeight_ << "px\","
	 <<   "cssClass: \"jp-video-" << videoHeight_ << "p\""
	 << "},";
    }

    ss << "cssSelectorAncestor: " << (gui_ ? "'#" + id() + '\'' : "''")
       << ", cssSelector: {";

    const char *controlSelectors[] = 
      { "videoPlay", "play", "pause", "stop", "volumeMute", "volumeUnmute",
	"volumeMax",
	"fullScreen", "restoreScreen", "repeat", "repeatOff" };

    first = true;
    for (unsigned i = VideoPlay; i < RepeatOff; ++i) {
      if (control_[i]) {
	if (!first)
	  ss << ", ";

	ss << const_cast<char *>(controlSelectors[i]) << ":\"#" 
	   << control_[i]->id() << "\"";

	first = false;
      }
    }

    const char *displaySelectors[] = 
      { "currentTime", "duration" };

    for (unsigned i = CurrentTime; i < Duration; ++i) {
      if (control_[i]) {
	if (!first)
	  ss << ", ";

	ss << const_cast<char *>(displaySelectors[i]) << ":\"#"
	   << display_[i]->id() << "\"";

	first = false;
      }
    }

    if (progressBar_[Time]) {
      if (!first)
	ss << ", ";

      ss << "seekBar:\"#" << progressBar_[Time]->id() << "\", "
	 << "playBar:\"#bar" << progressBar_[Time]->id() << "\"";

      first = false;
    }

    if (progressBar_[Volume]) {
      if (!first)
	ss << ", ";

      ss << "volumeBar:\"#" << progressBar_[Volume]->id() << "\", "
	 << "volumeBarValue:\"#bar" << progressBar_[Volume]->id() << "\"";

      first = false;
    }

    ss << '}'
       << "});";

    ss << "new " WT_CLASS ".WMediaPlayer("
       << app->javaScriptClass() << ',' << jsRef() << ");";

    doJavaScript(ss.str());

    boundSignals_ = 0;
  }

  if (boundSignals_ < signals_.size()) {
    WStringStream ss;
    ss << jsPlayerRef();
    for (unsigned i = boundSignals_; i < signals_.size(); ++i)
      ss << ".bind('" << signals_[i]->name() << "', function(o, e) { "
	 << signals_[i]->createCall() << "})";
    ss << ';';

    doJavaScript(ss.str());
    boundSignals_ = signals_.size();
  }

  WCompositeWidget::render(flags);
}

void WMediaPlayer::setFormData(const FormData& formData)
{
  if (!Utils::isEmpty(formData.values)) {
    std::vector<std::string> attributes;
    boost::split(attributes, formData.values[0], boost::is_any_of(";"));
    if (attributes.size() == 8) {
      try {
        status_.volume = boost::lexical_cast<double>(attributes[0]);
	status_.currentTime = boost::lexical_cast<double>(attributes[1]);
        status_.duration = boost::lexical_cast<double>(attributes[2]);
        status_.playing = (attributes[3] == "0");
        status_.ended = (attributes[4] == "1");
        status_.readyState
	  = intToReadyState(boost::lexical_cast<int>(attributes[5]));
	status_.playbackRate = boost::lexical_cast<double>(attributes[6]);
        status_.seekPercent = boost::lexical_cast<double>(attributes[7]);

	updateProgressBarState(Time);
	updateProgressBarState(Volume);

      } catch (const std::exception& e) {
	throw WException("WMediaPlayer: error parsing: "
			 + formData.values[0] + ": " + e.what());
      }
    } else
      throw WException("WMediaPlayer: error parsing: " + formData.values[0]);
  }
}

/*
JSignal<>& WMediaPlayer::loadStarted()
{ 
  return signal(LOAD_STARTED_SIGNAL);
}
*/

JSignal<>& WMediaPlayer::timeUpdated()
{
  return signal(TIME_UPDATED_SIGNAL);
}

JSignal<>& WMediaPlayer::playbackStarted()
{
  return signal(PLAYBACK_STARTED_SIGNAL);
}

JSignal<>& WMediaPlayer::playbackPaused()
{
  return signal(PLAYBACK_PAUSED_SIGNAL);
}

JSignal<>& WMediaPlayer::ended()
{
  return signal(ENDED_SIGNAL);
}

JSignal<>& WMediaPlayer::volumeChanged()
{
  return signal(VOLUME_CHANGED_SIGNAL);
}

JSignal<>& WMediaPlayer::signal(const char *name)
{
  for (unsigned i = 0; i < signals_.size(); ++i) {
    if (signals_[i]->name() == name)
      return *signals_[i];
  }

  JSignal<> *result;
  signals_.push_back(result = new JSignal<>(this, name, true));

  scheduleRender();

  return *result;
}

void WMediaPlayer::createDefaultGui()
{
  gui_ = 0;

  static const char *media[] = { "audio", "video" };

  WTemplate *ui = new WTemplate
    (tr(std::string("Wt.WMediaPlayer.defaultgui-") + media[mediaType_]));

  addAnchor(ui, Play, "play-btn", "jp-play");
  addAnchor(ui, Pause, "pause-btn", "jp-pause");
  addAnchor(ui, Stop, "stop-btn", "jp-stop");
  addAnchor(ui, VolumeMute, "mute-btn", "jp-mute");
  addAnchor(ui, VolumeUnmute, "unmute-btn", "jp-unmute");
  addAnchor(ui, VolumeMax, "volume-max-btn", "jp-volume-max");
  addAnchor(ui, RepeatOn, "repeat-btn", "jp-repeat");
  addAnchor(ui, RepeatOff, "repeat-off-btn", "jp-repeat-off");

  if (mediaType_ == Video) {
    addAnchor(ui, VideoPlay, "video-play-btn", "jp-video-play-icon", "play");
    addAnchor(ui, FullScreen, "full-screen-btn", "jp-full-screen");
    addAnchor(ui, RestoreScreen, "restore-screen-btn", "jp-restore-screen");
  }

  addText(ui, CurrentTime, "current-time", "jp-current-time");
  addText(ui, Duration, "duration", "jp-duration");
  addText(ui, Title, "title", std::string());

  addProgressBar(ui, Time, "progress-bar", "jp-seek-bar",
		 "jp-play-bar");
  addProgressBar(ui, Volume, "volume-bar", "jp-volume-bar",
		 "jp-volume-bar-value");

  ui->bindString("title-display", title_.empty() ? "none" : "");

  addStyleClass(mediaType_ == Video ? "jp-video" : "jp-audio");

  setControlsWidget(ui);
}

void WMediaPlayer::addAnchor(WTemplate *t, ButtonControlId id,
			     const char *bindId,
			     const std::string& styleClass,
			     const std::string& altText)
{
  std::string text;

  if (altText.empty())
    text = styleClass.substr(3).c_str();
  else
    text = altText;

  text = "Wt.WMediaPlayer." + text;

  WAnchor *anchor = new WAnchor(WLink("javascript:;"), WString::tr(text));
  anchor->setStyleClass(styleClass);
  anchor->setAttributeValue("tabindex", "1");
  anchor->setToolTip(WString::tr(text));
  anchor->setInline(false);

  t->bindWidget(bindId, anchor);

  setButton(id, anchor);
}

void WMediaPlayer::addText(WTemplate *t, TextId id, const char *bindId,
			   const std::string& styleClass)
{
  WText *text = new WText();
  text->setInline(false);

  if (!styleClass.empty())
    text->setStyleClass(styleClass);

  t->bindWidget(bindId, text);

  setText(id, text);
}

void WMediaPlayer::addProgressBar(WTemplate *t, BarControlId id,
				  const char *bindId,
				  const std::string& styleClass,
				  const std::string& valueStyleClass)
{
  WProgressBar *progressBar = new WProgressBar();
  progressBar->setStyleClass(styleClass);
  progressBar->setValueStyleClass(valueStyleClass);
  progressBar->setInline(false);

  t->bindWidget(bindId, progressBar);

  setProgressBar(id, progressBar);
}

}
