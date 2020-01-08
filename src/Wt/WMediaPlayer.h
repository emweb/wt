// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WMEDIA_PLAYER_H_
#define WMEDIA_PLAYER_H_

#include <Wt/WCompositeWidget.h>
#include <Wt/WJavaScript.h>
#include <Wt/WLink.h>
#include <Wt/WString.h>

namespace Wt {

/*! \brief An enumeration for a media encoding.
 *
 * \sa addSource()
 * \sa http://www.jplayer.org/latest/developer-guide/#jPlayer-media-encoding
 */
enum class MediaEncoding {
  PosterImage, //!< The poster image (e.g. JPG/PNG) for a video

  MP3,         //!< Audio: MP3 encoding (<b>essential audio</b> format)
  M4A,         //!< Audio: MP4 encoding (<b>essential audio</b> format)
  OGA,         //!< Audio: OGG encoding
  WAV,         //!< Audio: WAV (uncompressed) format
  WEBMA,       //!< Audio: WebM encoding
  FLA,         //!< Audio: Flash format

  M4V,         //!< Video: MP4 encoding (<b>essential video</b> format)
  OGV,         //!< Video: OGG encoding
  WEBMV,       //!< Video: WebM encoding
  FLV          //!< Video: Flash format
};

/*! \brief An enumeration for a media type.
 */
enum class MediaType {
  Audio,       //!< Defines an audio player
  Video        //!< Defines a video player
};

/*! \brief An enumeration for a button function.
 *
 * \sa setButton(), button()
 */
enum class MediaPlayerButtonId {
  VideoPlay,     //!< Play button which overlays the video (for Video only)
  Play,          //!< Play button, is hidden while playing
  Pause,         //!< Pause button, is hidden while paused
  Stop,          //!< Stop button
  VolumeMute,    //!< Volume mute button
  VolumeUnmute,  //!< Volume unmute button
  VolumeMax,     //!< Volume max button

  /*! Toggle button for full screen, is hidden while full screen
   * (for Video only) */
  FullScreen,
  /*! Toggle button to restore the screen, is shown only in full screen
   * (for Video only) */
  RestoreScreen,

  /*! Toggle button to enable looping, is hidden while repeating is on */
  RepeatOn,
  /*! Toggle button to disable looping, is hidden while repeat is off */
  RepeatOff
};

/*! \brief An enumeration for a progressbar function.
 *
 * \sa setProgressBar(), progressBar()
 */
enum class MediaPlayerProgressBarId {
  Time,  //!< The time bar
  Volume //!< The volume bar
};

/*! \brief An enumeration for a text.
 *
 * \sa setText(), text()
 */
enum class MediaPlayerTextId {
  CurrentTime, //!< Displays the current time
  Duration,    //!< Displays the total duration
  Title        //!< Displays the title set in setTitle()
};

/*! \class WMediaPlayer Wt/WMediaPlayer.h Wt/WMediaPlayer.h
 *  \brief A media player
 *
 * This widget implements a media player, suitable to play video or
 * audio, and with a customizable user-interface.
 *
 * To support cross-browser playing of video or audio content, you may
 * need to provide the contents appropriately encoded. For audio, at
 * least an MP3 or MP4 audio (M4A) encoding should be supplied, while
 * for video the M4V encoding should be provided. Additional encodings
 * are beneficial since they increase the chance that native HTML
 * <tt>&lt;video&gt;</tt> or <tt>&lt;audio&gt;</tt> elements can be
 * used (which may be hardware accelerated), instead of the flash
 * player. See <a
 * href="http://www.jplayer.org/latest/developer-guide/#reference-html5-media">
 * HTML5 browser media support</a>.
 *
 * You need to specify the encoding types you are going to use when
 * instantiating the media player, since based on the chosen
 * encodings, a particular suitable implementation will be used. Thus,
 * you need to call addSource() immediately, but you may pass empty
 * URLs if you do not yet want to load media.
 *
 * The player provides a user-interface to control the playback which
 * may be freely customized, and which is independent of the
 * underlying media technology (HTML video or Flash player). The
 * controls user-interface may be implemented as a %Wt widget, where
 * the controls (buttons, progress bars, and text widgets) are bound
 * directly to the video player component (client-side).
 *
 * This widget relies on a third-party JavaScript component <a
 * href="http://www.jplayer.org/">jPlayer</a>, which is
 * distributed together with %Wt.
 *
 * The default user-interface can be themed using jPlayer themes. The
 * theme is global (it applies to all media player instances), and is
 * configured by loading a CSS stylesheet.
 *
 * The following code creates a video using the default controls:
 * \if cpp
 * \code
 *   useStyleSheet(WApplication::relativeResourcesUrl() + "jPlayer/skin/jplayer.blue.monday.css");
 *
 *   auto player = std::make_unique<WMediaPlayer>(MediaType::Video);
 *
 *   player->addSource(MediaEncoding::M4V, "video.m4v");
 *   player->addSource(MediaEncoding::OGV, "video.ogv");
 *   player->addSource(MediaEncoding::PosterImage, "poster.png");
 *
 * \endcode
 * \elseif java
 * \code
 * ...
 * \endcode
 * \endif
 *
 * Alternatively, a custom widget may be set which implements the
 * controls, using setControlsWidget(). In this case, you should add
 * to this widget the buttons, text place holders, and progress bars
 * and bind them to the media player using the setButton(), setText()
 * and setProgressBar() methods. The controls widget is integrated in
 * the media player, and this has as unique benefit (for a video
 * player) that they may also be shown when the video player is
 * maximized.
 *
 * Finally, you may want to control the media player only through
 * widgets external to the media player. This may be configured by
 * setting \c 0 as controlsWidget. In this case however, full screen
 * mode should not be used since there is no way to restore the
 * original size.
 */
class WT_API WMediaPlayer : public WCompositeWidget
{
public:
  /*! \brief Typedef for enum Wt::MediaEncoding */
  typedef MediaEncoding Encoding;
  /*! \brief Typedef for enum Wt::MediaType */
  typedef MediaType Type;
  /*! \brief Typedef for enum Wt::MediaPlayerButtonId */
  typedef MediaPlayerButtonId ButtonId;
  /*! \brief Typedef for enum Wt::MediaPlayerProgressBarId */
  typedef MediaPlayerProgressBarId ProgressBarId;
  /*! \brief Typedef for enum Wt::MediaPlayerTextId */
  typedef MediaPlayerTextId TextId;

  /*! \brief Creates a new media player.
   *
   * The player is instantiated with default controls.
   *
   * \sa setControlsWidget()
   */
  WMediaPlayer(MediaType mediaType);

  /*! \brief Destructor.
   */
  ~WMediaPlayer();

  /*! \brief Sets the video size.
   *
   * This sets the size for the video. The actual size of the media
   * player may be slightly larger, if the controlWidget take
   * additional space (i.e. is not overlayed on top of the video).
   *
   * CSS Themes for the default jPlayer controls support two formats
   * (480 x 270 and 640 x 360).
   *
   * The default video size is 480 x 270.
   */
  void setVideoSize(int width, int height);

  /*! \brief Returns the video width.
   *
   * \sa setVideoSize()
   */
  int videoWidth() const { return videoWidth_; }

  /*! \brief Returns the video height.
   *
   * \sa setVideoSize()
   */
  int videoHeight() const { return videoHeight_; }

  /*! \brief Sets the user-interface controls widget.
   *
   * This sets a widget that contains the controls (buttons, text
   * widgets, etc...) to allow the user to control the player.
   *
   * Widgets that implement the buttons, bars, and text holders should
   * be bound to the player using setButton(), setText() and
   * setProgressBar() calls.
   *
   * Setting a \c 0 widget will result in a player without
   * controls. For an audio player this has the effect of being
   * entirely invisible.
   *
   * The default controls widget is a widget that can be styled using
   * a jPlayer CSS theme.
   */
  void setControlsWidget(std::unique_ptr<WWidget> controls);

  /*! \brief Returns the user-interface controls widget.
   *
   * \sa setControlsWidget()
   */
  WWidget *controlsWidget() const;

  /*! \brief Sets the media title.
   *
   * \sa MediaPlayerTextId::Title
   */
  void setTitle(const WString& title);

  /*! \brief Adds a source.
   *
   * Adds a media source. The source may be specified as a URL or as a
   * dynamic resource.
   *
   * You may pass a null \p link if you want to indicate the media types
   * you will use (later) without already loading data.
   */
  void addSource(MediaEncoding encoding, const WLink& link);

  /*! \brief Returns a source.
   *
   * Returns the media source for the given \p encoding, which must have
   * previously been added using addSource().
   */
  WLink getSource(MediaEncoding encoding) const;

  /*! \brief Clears all sources.
   *
   * \sa addSource()
   */
  void clearSources();

  /*! \brief Binds a control button.
   *
   * A control button is typically implemented as a WAnchor or a
   * WPushButton (although any WInteractWidget can work).
   *
   * You should use this method in conjunction with
   * setControlsWidget() to bind buttons in a custom control interface
   * to media player functions.
   *
   * The default control widget implements all buttons using a
   * WAnchor.
   */
  void setButton(MediaPlayerButtonId id, WInteractWidget *btn);

  /*! \brief Returns a control button.
   *
   * \sa setButton()
   */
  WInteractWidget *button(MediaPlayerButtonId id) const;

  /*! \brief Binds a control progress bar.
   *
   * The progress bar for the MediaPlayerProgressBarId::Time
   * indication should be contained in a WContainerWidget which bounds
   * the width of the progress bar, rather than setting a width on the
   * progress bar. This is because the progress bar may, in some
   * cases, also be used to indicate which part of the media can be
   * seeked, and for this its width is being manipulated.
   *
   * You should use this method in conjunction with
   * setControlsWidget() to bind progress bars in a custom control
   * interface to media player functions.
   */
  void setProgressBar(MediaPlayerProgressBarId id, WProgressBar *progressBar);

  /*! \brief Returns a control progress bar.
   *
   * \sa setProgressBar()
   */
  WProgressBar *progressBar(MediaPlayerProgressBarId id) const;

  /*! \brief Sets a text place-holder widget.
   *
   * This binds the widget that displays text such as current time and
   * total duration of the loaded media.
   *
   * You should use this method in conjunction with
   * setControlsWidget() to bind progress bars in a custom control
   * interface to media player functions.
   */
  void setText(MediaPlayerTextId id, WText *text);

  /*! \brief Returns a text place-holder widget.
   *
   * \sa setText()
   */
  WText *text(MediaPlayerTextId id) const;

  /*! \brief Pauses the player.
   *
   * \sa play()
   */
  void pause();

  /*! \brief Start or resume playing.
   *
   * The player starts or resumes playing at the current time.
   *
   * \sa seek()
   */
  void play();

  /*! \brief Stops the player.
   *
   * \sa play()
   */
  void stop();

  /*! \brief Seeks to a time.
   *
   * If possible, the player sets the current time to the indicated \p time
   * (expressed in seconds).
   *
   * \note It may be the case that this only works after the player has
   *       already loaded the media.
   */
  void seek(double time);

  /*! \brief Sets the playback rate.
   *
   * This modifies the playback rate, expressed as a ratio of the
   * normal (natural) playback rate.
   *
   * The default value is 1.0
   *
   * \note Not all browsers support this function.
   */
  void setPlaybackRate(double rate);

  /*! \brief Sets the volume.
   *
   * This modifies the volume, which must be a number between 0 and 1.0.
   *
   * The default value is 0.8
   */
  void setVolume(double volume);

  /*! \brief Returns the volume.
   *
   * \sa setVolume()
   */
  double volume() const;

  /*! \brief Mutes or unmutes the playback volume.
   *
   * \sa setVolume() 
   */
  void mute(bool mute);

  /*! \brief Returns whether the media is currently playing.
   *
   * \sa play()
   */
  bool playing() const { return status_.playing; }

  /*! \brief Returns the current player state.
   *
   * The state reflects in how far the media player has loaded the
   * media, and has determined its characteristics.
   *
   */
  MediaReadyState readyState() const { return status_.readyState; }

  /*! \brief Returns the duration.
   *
   * The duration may be reported as 0 if the player has not yet loaded
   * the media to determine the duration. Otherwise the duration is the
   * duration of the loaded media, expressed in seconds.
   *
   * \sa readyState(), currentTime()
   */
  double duration() const { return status_.duration; }

  /*! \brief Returns the current playback time.
   *
   * Returns the current playback time, expressed in seconds.
   *
   * \sa seek()
   */
  double currentTime() const { return status_.currentTime; }

  /*! \brief Returns the current playback rate.
   *
   * \sa setPlaybackRate()
   */
  double playbackRate() const { return status_.playbackRate; }

  // This even seems kind of useless ?
  // JSignal<>& loadStarted();

  /*! \brief Event that indicates a time update.
   *
   * The event indicates that the currentTime() has changed.
   */
  JSignal<double>& timeUpdated();

  /*! \brief Event that indicates that playback started.
   *
   * The event is fired when playback has started (or is being
   * continued).
   */
  JSignal<>& playbackStarted();

  /*! \brief Event that indicates that playback paused.
   *
   * The event is fired when playback has been paused.
   */
  JSignal<>& playbackPaused();

  /*! \brief Event that indicates that the video or audio has ended.
   */
  JSignal<>& ended();

  /*! \brief Event that indicates that the volume has changed.
   */
  JSignal<double>& volumeChanged();

  std::string jsPlayerRef() const;

  virtual void refresh() override;

protected:
  virtual void setFormData(const FormData& formData) override;
  virtual void render(WFlags<RenderFlag> flags) override;

private:
  struct Source {
    MediaEncoding encoding;
    WLink link;
  };

  struct SignalDouble {
    JSignal<double> *signal;
    std::string jsExprA1;
  };

  std::vector<JSignal<> *> signals_;
  std::vector<SignalDouble> signalsDouble_;

  MediaType mediaType_;
  int videoWidth_, videoHeight_;

  WString title_;
  std::vector<Source> media_;
  std::string initialJs_;
 
  observing_ptr<WInteractWidget> control_[11];
  WText *display_[3];
  WProgressBar *progressBar_[2];

  observing_ptr<WWidget> gui_;
  unsigned boundSignals_, boundSignalsDouble_;

  bool mediaUpdated_;

  struct State {
    bool playing, ended;
    MediaReadyState readyState;
    double seekPercent, volume, duration, currentTime, playbackRate;

    State();
  };

  State status_;

  void createDefaultGui();

  void addAnchor(WTemplate *t, MediaPlayerButtonId id, const char *bindId,
		 const std::string& styleClass,
		 const std::string& altText = std::string());
  void addText(WTemplate *t, MediaPlayerTextId id, const char *bindId,
	       const std::string& styleClass);
  void addProgressBar(WTemplate *t, MediaPlayerProgressBarId id,
		      const char *bindId,
		      const std::string& styleClass,
		      const std::string& valueStyleClass);
  JSignal<>& signal(const char *name);
  JSignal<double>& signalDouble(const char *name, const std::string& expr);

  void updateProgressBarState(MediaPlayerProgressBarId id);
  void updateFromProgressBar(MediaPlayerProgressBarId id, double value);

  void playerDo(const std::string& method,
		const std::string& args = std::string());
  void playerDoData(const std::string& method, const std::string& args); 
  void playerDoRaw(const std::string& jqueryMethod);

  static const char *LOAD_STARTED_SIGNAL;
  static const char *TIME_UPDATED_SIGNAL;
  static const char *PLAYBACK_STARTED_SIGNAL;
  static const char *PLAYBACK_PAUSED_SIGNAL;
  static const char *ENDED_SIGNAL;
  static const char *VOLUME_CHANGED_SIGNAL;

  friend class WMediaPlayerImpl;
};

}

#endif // WMEDIA_PLAYER_H_

