// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WMediaPlayer.h"
#include "Wt/WAnchor.h"
#include "Wt/WApplication.h"
#include "Wt/WEnvironment.h"
#include "Wt/WException.h"
#include "Wt/WProgressBar.h"
#include "Wt/WStringStream.h"
#include "Wt/WTemplate.h"
#include "Wt/WText.h"
#include "WebUtils.h"

#ifndef WT_DEBUG_JS
#include "js/WMediaPlayer.min.js"
#endif

#include <boost/algorithm/string.hpp>

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

namespace Wt {

const char *WMediaPlayer::LOAD_STARTED_SIGNAL = "jPlayer_loadstart.Wt";
const char *WMediaPlayer::TIME_UPDATED_SIGNAL = "jPlayer_timeupdate.Wt";
const char *WMediaPlayer::PLAYBACK_STARTED_SIGNAL = "jPlayer_play.Wt";
const char *WMediaPlayer::PLAYBACK_PAUSED_SIGNAL = "jPlayer_pause.Wt";
const char *WMediaPlayer::ENDED_SIGNAL = "jPlayer_ended.Wt";
const char *WMediaPlayer::VOLUME_CHANGED_SIGNAL = "jPlayer_volumechange.Wt";

class WMediaPlayerImpl final : public WTemplate
{
public:
  WMediaPlayerImpl(WMediaPlayer *player, const WString& text)
    : WTemplate(text),
      player_(player)
  { 
    setFormObject(true);
  }

protected:
  virtual std::string renderRemoveJs(bool recursive) override
  {
    if (isRendered()) {
      std::string result = player_->jsPlayerRef() + ".jPlayer('destroy');";

      if (!recursive)
	result += WT_CLASS ".remove('" + id() + "');";

      return result;
    } else
      return WTemplate::renderRemoveJs(recursive);
  }

  virtual void setFormData(const FormData& formData) override
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
    readyState(MediaReadyState::HaveNothing),
    seekPercent(0),
    volume(0.8),
    duration(0),
    currentTime(0),
    playbackRate(1)
{ }

WMediaPlayer::WMediaPlayer(MediaType mediaType)
  : mediaType_(mediaType),
    videoWidth_(0),
    videoHeight_(0),
    gui_(this),
    boundSignals_(0),
    boundSignalsDouble_(0)
{
  for (unsigned i = 0; i < 11; ++i)
    control_[i] = nullptr;

  for (unsigned i = 0; i < 3; ++i)
    display_[i] = nullptr;

  for (unsigned i = 0; i < 2; ++i)
    progressBar_[i] = nullptr;

  std::unique_ptr<WTemplate> impl
    (new WMediaPlayerImpl(this, tr("Wt.WMediaPlayer.template")));
  impl->bindEmpty("gui");
  setImplementation(std::move(impl));

  WApplication *app = WApplication::instance();

  LOAD_JAVASCRIPT(app, "js/WMediaPlayer.js", "WMediaPlayer", wtjs1);

  std::string res = WApplication::relativeResourcesUrl() + "jPlayer/";

  if (!app->environment().ajax())
    app->require(res + "jquery.min.js");

  if (app->require(res + "jquery.jplayer.min.js"))
    app->useStyleSheet(res + "skin/jplayer.blue.monday.css");

  if (mediaType_ == MediaType::Video)
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
  for (unsigned i = 0; i < signals_.size(); ++i)
    delete signals_[i];

  for (unsigned i = 0; i < signalsDouble_.size(); ++i)
    delete signalsDouble_[i].signal;
}

void WMediaPlayer::clearSources()
{
  media_.clear();

  mediaUpdated_ = true;
  scheduleRender();
}

void WMediaPlayer::addSource(MediaEncoding encoding, const WLink& link)
{
  media_.push_back(Source());
  media_.back().link = link;
  media_.back().encoding = encoding;

  mediaUpdated_ = true;
  scheduleRender();
}

WLink WMediaPlayer::getSource(MediaEncoding encoding) const
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

  if (display_[static_cast<unsigned int>(MediaPlayerTextId::Title)]) {
    display_[static_cast<unsigned int>(MediaPlayerTextId::Title)]
      ->setText(title_);

    if (gui_) {
      WTemplate *t = dynamic_cast<WTemplate *>(gui_.get());
      if (t)
	t->bindString("title-display", title_.empty() ? "none" : "");
    }
  }
}

void WMediaPlayer::setControlsWidget(std::unique_ptr<WWidget> controlsWidget)
{
  gui_ = controlsWidget.get();

  WTemplate *impl = dynamic_cast<WTemplate *>(implementation());

  if (controlsWidget) {
    controlsWidget->addStyleClass("jp-gui");

    impl->bindWidget("gui", std::move(controlsWidget));
  } else
    impl->bindEmpty("gui");
}

WWidget *WMediaPlayer::controlsWidget() const
{
  if (gui_.get() == this)
    (const_cast<WMediaPlayer *>(this))->createDefaultGui();

  return gui_.get();
}

void WMediaPlayer::setButton(MediaPlayerButtonId id, WInteractWidget *w)
{
  if (control_[static_cast<unsigned int>(id)])
    control_[static_cast<unsigned int>(id)]->parent()
      ->removeWidget(control_[static_cast<unsigned int>(id)].get());
  control_[static_cast<unsigned int>(id)] = w;
}

WInteractWidget *WMediaPlayer::button(MediaPlayerButtonId id) const
{
  controlsWidget(); // may lazy-create the default gui.
  return control_[static_cast<unsigned int>(id)].get();
}

void WMediaPlayer::setText(MediaPlayerTextId id, WText *w)
{
  delete display_[static_cast<unsigned int>(id)];
  display_[static_cast<unsigned int>(id)] = w;

  if (id == MediaPlayerTextId::Title && w)
    w->setText(title_);
}

WText *WMediaPlayer::text(MediaPlayerTextId id) const
{
  controlsWidget(); // may lazy-create the default gui.

  return display_[static_cast<unsigned int>(id)];
}

void WMediaPlayer::setProgressBar(MediaPlayerProgressBarId id, WProgressBar *w)
{
  const MediaPlayerProgressBarId bc_id = id;

  delete progressBar_[static_cast<unsigned int>(id)];
  progressBar_[static_cast<unsigned int>(id)] = w;

  if (w) {
    w->setFormat(WString::Empty);

    w->valueChanged().connect
      (this, std::bind(&WMediaPlayer::updateFromProgressBar, this, bc_id,
		       std::placeholders::_1));

    updateProgressBarState(id);
  }
}

void WMediaPlayer::updateProgressBarState(MediaPlayerProgressBarId id)
{
  WProgressBar *bar = progressBar(id);
  if (bar) {
    switch (id) {
    case MediaPlayerProgressBarId::Time:
      bar->setState(0, status_.seekPercent * status_.duration,
		    status_.currentTime);
      break;
    case MediaPlayerProgressBarId::Volume:
      bar->setState(0, 1, status_.volume);
    }
  }
}

void WMediaPlayer::updateFromProgressBar(MediaPlayerProgressBarId id,
					 double value)
{
  switch (id) {
  case MediaPlayerProgressBarId::Time:
    seek(value); break;
  case MediaPlayerProgressBarId::Volume:
    setVolume(value);
  } 
}

WProgressBar *WMediaPlayer::progressBar(MediaPlayerProgressBarId id) const
{
  controlsWidget(); // may lazy-create the default gui.

  return progressBar_[static_cast<unsigned int>(id)];
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
  playerDo("volume", std::to_string(volume));
}

double WMediaPlayer::volume() const
{
  return status_.volume;
}

void WMediaPlayer::setPlaybackRate(double rate)
{
  if (rate != status_.playbackRate) {
    status_.playbackRate = rate;

    playerDoData("wtPlaybackRate", std::to_string(rate));
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

    playerDo("playHead", std::to_string(pct * 100));
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
  render(RenderFlag::Full);
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

      ss << const_cast<char *>
	(mediaNames[static_cast<unsigned int>(media_[i].encoding)]) << ": "
	 << WWebWidget::jsStringLiteral(url);

      first = false;
    }

    ss << '}';

    if (!(flags & RenderFlag::Full))
      playerDo("setMedia", ss.str());
    else {
      initialJs_ = ".jPlayer('setMedia', " + ss.str() + ')' + initialJs_;
    }

    mediaUpdated_ = false;
  }

  if (flags.test(RenderFlag::Full)) {
    if (gui_.get() == this)
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
      if (media_[i].encoding != MediaEncoding::PosterImage) {
	if (!first)
	  ss << ',';
	ss << const_cast<char *>
	  (mediaNames[static_cast<unsigned int>(media_[i].encoding)]);
	first = false;
      }
    }

    ss << "\",";

    if (mediaType_ == MediaType::Video) {
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
    for (unsigned i = static_cast<unsigned int>
	   (MediaPlayerButtonId::VideoPlay); 
	 i < static_cast<unsigned int>(MediaPlayerButtonId::RepeatOff); ++i) {
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

    for (unsigned i = static_cast<unsigned int>(MediaPlayerTextId::CurrentTime);
	 i < static_cast<unsigned int>(MediaPlayerTextId::Duration); ++i) {
      if (control_[i]) {
	if (!first)
	  ss << ", ";

	ss << const_cast<char *>(displaySelectors[i]) << ":\"#"
	   << display_[i]->id() << "\"";

	first = false;
      }
    }

    if (progressBar_[static_cast<unsigned int>
		     (MediaPlayerProgressBarId::Time)]) {
      if (!first)
	ss << ", ";

      ss << "seekBar:\"#"
	 << progressBar_[static_cast<unsigned int>
			 (MediaPlayerProgressBarId::Time)]->id()
	 << "\", "
	 << "playBar:\"#bar"
	 << progressBar_[static_cast<unsigned int>
			 (MediaPlayerProgressBarId::Time)]->id()
	 << "\"";

      first = false;
    }

    if (progressBar_[static_cast<unsigned int>
		     (MediaPlayerProgressBarId::Volume)]) {
      if (!first)
	ss << ", ";

      ss << "volumeBar:\"#"
	 << progressBar_[static_cast<unsigned int>
			 (MediaPlayerProgressBarId::Volume)]->id()
	 << "\", "
	 << "volumeBarValue:\"#bar" 
	 << progressBar_[static_cast<unsigned int>
			 (MediaPlayerProgressBarId::Volume)]->id()
	 << "\"";

      first = false;
    }

    ss << '}'
       << "});";

    ss << "new " WT_CLASS ".WMediaPlayer("
       << app->javaScriptClass() << ',' << jsRef() << ");";

    doJavaScript(ss.str());

    boundSignals_ = 0;
    boundSignalsDouble_ = 0;
  }

  if (boundSignals_ < signals_.size()) {
    WStringStream ss;
    ss << jsPlayerRef();
    for (unsigned i = boundSignals_; i < signals_.size(); ++i)
      ss << ".bind('" << signals_[i]->name() << "', function(o, e) { "
#ifndef WT_TARGET_JAVA
	 << signals_[i]->createCall({}) << "})";
#else
	 << signals_[i]->createCall() << "})";
#endif
    ss << ';';

    doJavaScript(ss.str());
    boundSignals_ = signals_.size();
  }

  if (boundSignalsDouble_ < signalsDouble_.size()) {
    WStringStream ss;
    ss << jsPlayerRef();
    for (unsigned i = boundSignalsDouble_; i < signalsDouble_.size(); ++i)
      ss << ".bind('" << signalsDouble_[i].signal->name()
	 << "', function(o, e) { "
#ifndef WT_TARGET_JAVA
	 << signalsDouble_[i].signal->createCall({signalsDouble_[i].jsExprA1})
#else
	 << signalsDouble_[i].signal->createCall(signalsDouble_[i].jsExprA1)
#endif
	 << "})";
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
        status_.volume = Utils::stod(attributes[0]);
	status_.currentTime = Utils::stod(attributes[1]);
        status_.duration = Utils::stod(attributes[2]);
        status_.playing = (attributes[3] == "0");
        status_.ended = (attributes[4] == "1");
        status_.readyState = intToReadyState(Utils::stoi(attributes[5]));
	status_.playbackRate = Utils::stod(attributes[6]);
        status_.seekPercent = Utils::stod(attributes[7]);

	updateProgressBarState(MediaPlayerProgressBarId::Time);
	updateProgressBarState(MediaPlayerProgressBarId::Volume);

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

JSignal<double>& WMediaPlayer::timeUpdated()
{
  return signalDouble(TIME_UPDATED_SIGNAL,
		      jsPlayerRef() + ".data('jPlayer').status.currentTime");
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

JSignal<double>& WMediaPlayer::volumeChanged()
{
  return signalDouble(VOLUME_CHANGED_SIGNAL,
		      jsPlayerRef() + ".data('jPlayer').options.volume");
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

JSignal<double>& WMediaPlayer::signalDouble(const char *name,
					    const std::string& jsExpr)
{
  for (unsigned i = 0; i < signalsDouble_.size(); ++i) {
    if (signalsDouble_[i].signal->name() == name)
      return *signalsDouble_[i].signal;
  }

  SignalDouble sd;
  sd.signal = new JSignal<double>(this, name, true);
  sd.jsExprA1 = jsExpr;
  signalsDouble_.push_back(sd);

  scheduleRender();

  return *sd.signal;
}

void WMediaPlayer::createDefaultGui()
{
  gui_ = nullptr;

  static const char *media[] = { "audio", "video" };

  std::unique_ptr<WTemplate> ui
    (new WTemplate(tr(std::string("Wt.WMediaPlayer.defaultgui-")
		      + media[static_cast<unsigned int>(mediaType_)])));

  addAnchor(ui.get(), MediaPlayerButtonId::Play, "play-btn", "jp-play");
  addAnchor(ui.get(), MediaPlayerButtonId::Pause, "pause-btn", "jp-pause");
  addAnchor(ui.get(), MediaPlayerButtonId::Stop, "stop-btn", "jp-stop");
  addAnchor(ui.get(), MediaPlayerButtonId::VolumeMute, "mute-btn", "jp-mute");
  addAnchor(ui.get(), MediaPlayerButtonId::VolumeUnmute, "unmute-btn", "jp-unmute");
  addAnchor(ui.get(), MediaPlayerButtonId::VolumeMax, "volume-max-btn",
	    "jp-volume-max");
  addAnchor(ui.get(), MediaPlayerButtonId::RepeatOn, "repeat-btn", "jp-repeat");
  addAnchor(ui.get(), MediaPlayerButtonId::RepeatOff, "repeat-off-btn", "jp-repeat-off");

  if (mediaType_ == MediaType::Video) {
    addAnchor(ui.get(), MediaPlayerButtonId::VideoPlay, "video-play-btn",
	      "jp-video-play-icon", "play");
    addAnchor(ui.get(), MediaPlayerButtonId::FullScreen, "full-screen-btn",
	      "jp-full-screen");
    addAnchor(ui.get(), MediaPlayerButtonId::RestoreScreen, "restore-screen-btn",
	      "jp-restore-screen");
  }

  addText(ui.get(), MediaPlayerTextId::CurrentTime, 
	  "current-time", "jp-current-time");
  addText(ui.get(), MediaPlayerTextId::Duration, "duration", "jp-duration");
  addText(ui.get(), MediaPlayerTextId::Title, "title", std::string());

  addProgressBar(ui.get(), MediaPlayerProgressBarId::Time, "progress-bar", "jp-seek-bar",
		 "jp-play-bar");
  addProgressBar(ui.get(), MediaPlayerProgressBarId::Volume, "volume-bar", "jp-volume-bar",
		 "jp-volume-bar-value");

  ui->bindString("title-display", title_.empty() ? "none" : "");

  addStyleClass(mediaType_ == MediaType::Video ? "jp-video" : "jp-audio");

  setControlsWidget(std::move(ui));
}

void WMediaPlayer::addAnchor(WTemplate *t, MediaPlayerButtonId id,
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

  std::unique_ptr<WAnchor> anchor
    (new WAnchor(WLink("javascript:;"), WString::tr(text)));
  anchor->setStyleClass(styleClass);
  anchor->setAttributeValue("tabindex", "1");
  anchor->setToolTip(WString::tr(text));
  anchor->setInline(false);

  setButton(id, anchor.get());

  t->bindWidget(bindId, std::move(anchor));
}

void WMediaPlayer::addText(WTemplate *t, MediaPlayerTextId id,
			   const char *bindId, const std::string& styleClass)
{
  std::unique_ptr<WText> text(new WText());
  text->setInline(false);

  if (!styleClass.empty())
    text->setStyleClass(styleClass);

  setText(id, text.get());

  t->bindWidget(bindId, std::move(text));
}

void WMediaPlayer::addProgressBar(WTemplate *t, MediaPlayerProgressBarId id,
				  const char *bindId,
				  const std::string& styleClass,
				  const std::string& valueStyleClass)
{
  std::unique_ptr<WProgressBar> progressBar(new WProgressBar());
  progressBar->setStyleClass(styleClass);
  progressBar->setValueStyleClass(valueStyleClass);
  progressBar->setInline(false);

  setProgressBar(id, progressBar.get());

  t->bindWidget(bindId, std::move(progressBar));
}

}
