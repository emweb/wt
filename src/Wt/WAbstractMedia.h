// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WABSTRACT_MEDIA_H_
#define WABSTRACT_MEDIA_H_

#include <Wt/WInteractWidget.h>
#include <Wt/WLink.h>
#include <Wt/WFlags.h>

namespace Wt {

/*! \brief Enumeration for playback options
 */
enum class PlayerOption {
  Autoplay = 1, //!< Start playing as soon as the video is loaded
  Loop     = 2, //!< Enable loop mode
  Controls = 4  //!< Show video controls in the browser
};

/*! \brief Enumeration for preload strategy
 */
enum class MediaPreloadMode {
  None,    //!< Hints that the user will probably not play the video
  Auto,    //!< Hints that it is ok to download the entire resource
  Metadata //!< Hints that retrieving metadata is a good option
};

/*! \class WAbstractMedia Wt/WAbstractMedia.h Wt/WAbstractMedia.h
 *  \brief Abstract baseclass for native media elements.
 *
 * This class is an abstract base class for HTML5 media elements
 * (&lt;audio&gt;, &lt;video&gt;).
 *
 */
class WT_API WAbstractMedia : public WInteractWidget
{
public:
  /*! \brief Typedef for enum Wt::PlayerOption */
  typedef PlayerOption Option;

  /*! \brief Typedef for enum Wt::MediaPreloadMode */
  typedef MediaPreloadMode PreloadMode;

  /*! \brief Typedef for enum Wt::MediaReadyState */
  typedef MediaReadyState ReadyState;
  
  /*! \brief Consctructor for a media widget.
   *
   * A freshly constructed media widget has no options set, no media
   * sources, and has preload mode set to PreloadAuto.
   */
  WAbstractMedia();

  ~WAbstractMedia();

  /*! \brief Set the media element options
   *
   * \sa Options
   */
  void setOptions(const WFlags<PlayerOption> &flags);

  /*! \brief Retrieve the configured options
   */
  WFlags<PlayerOption> getOptions() const;

  /*! \brief Set the preload mode
   */
  void setPreloadMode(MediaPreloadMode mode);

  /*! \brief Retrieve the preload mode
   */
  MediaPreloadMode preloadMode() const;

  /*! \brief Removes all source elements
   *
   * This method can be used to remove all media sources. Afterward, you
   * may add new media sources with calls to addSource().
   *
   * Use this to reuse a WAbstractMedia instantiation to play something else.
   */
  void clearSources();

  /*! \brief Add a media source
   *
   * This method specifies a media source (which may be a URL or
   * dynamic resource). You may add as many media sources as you
   * want. The browser will select the appropriate media stream to
   * display to the user.
   *
   * This method specifies a media source using the URL, the mime type,
   * and the media attribute. HTML allows for empty type and media
   * attributes.
   */
  void addSource(const WLink& source, const std::string &type = "",
		 const std::string &media = "");

  /*! \brief Content to be shown when media cannot be played
   *
   * As not all browsers are HTML5 compliant, it is a good idea to
   * provide fallback options when the media cannot be displayed.  If
   * the media can be played by the browser, the alternative content
   * will be suppressed.
   *
   * The two reasons to display the alternative content are (1) the
   * media tag is not supported, or (2) the media tag is supported, but
   * none of the media sources are supported by the browser. In the first
   * case, fall-back is automatic and does not rely on JavaScript in the
   * browser; in the latter case, JavaScript is required to make the
   * fallback work.
   *
   * The alternative content can be any widget: you can set it to an
   * alternative media player (QuickTime, Flash, ...), show a
   * Flash movie, an animated gif, a text, a poster image, ...
   */
  void setAlternativeContent(std::unique_ptr<WWidget> alternative);

  /*! \brief Invoke play() on the media element
   *
   * JavaScript must be available for this function to work.
   */
  void play();

  /*! \brief Invoke pause() on the media element
   *
   * JavaScript must be available for this function to work.
   */
  void pause();

  /*! \brief Returns whether the media is playing.
   */
  bool playing() const { return playing_; }

  /*! \brief Returns the media's readyState
   */
  MediaReadyState readyState() const { return readyState_; }

  /*! \brief Event signal emitted when playback has begun.
   *
   * This event fires when play was invoked, or when the media element
   * starts playing because the Autoplay option was provided.
   *
   * \note When JavaScript is disabled, the signal will never fire.
   */
  EventSignal<>& playbackStarted();

  /*! \brief Event signal emitted when the playback has paused.
   *
   * \note When JavaScript is disabled, the signal will never fire.
   */
  EventSignal<>& playbackPaused();

  /*! \brief Event signal emitted when the playback stopped because
   *         the end of the media was reached.
   *
   * \note When JavaScript is disabled, the signal will never fire.
   */
  EventSignal<>& ended();

  /*! \brief Event signal emitted when the current playback position has
   *         changed.
   *
   * This event is fired when the playback position has changed,
   * both when the media is in a normal playing mode, but also when it has
   * changed discontinuously because of another reason.
   *
   * \note When JavaScript is disabled, the signal will never fire.
   */
  EventSignal<>& timeUpdated();

  /*! \brief Event signal emitted when the playback volume has changed.
   *
   * \note When JavaScript is disabled, the signal will never fire.
   */
  EventSignal<>& volumeChanged();

  /*! \brief Returns the JavaScript reference to the media object, or null.
   *
   * It is possible, for browser compatibility reasons, that jsRef()
   * is not the media element. jsMediaRef() is guaranteed to be an
   * expression that evaluates to the media object. This expression
   * may yield null, if the video object is not rendered at all
   * (e.g. on older versions of Internet Explorer).
   */
  std::string jsMediaRef() const;

protected:
  virtual void getDomChanges(std::vector<DomElement *>& result,
			     WApplication *app) override;
  virtual DomElement *createDomElement(WApplication *app) override;
  virtual void iterateChildren(const HandleWidgetMethod& method) const override;

  virtual void updateMediaDom(DomElement& element, bool all);
  virtual DomElement *createMediaDomElement() = 0;

  virtual void setFormData(const FormData& formData) override;
  virtual void enableAjax() override;

private:
  struct Source 
#ifdef WT_TARGET_JAVA
    : public WObject
#endif
  {
    Source(WAbstractMedia *parent, const WLink& link, const std::string &type,
           const std::string &media);
    ~Source();

    void resourceChanged();

    WAbstractMedia *parent;
    Wt::Signals::connection connection;
    std::string type, media;
    WLink link;
  };
  void renderSource(DomElement* element, WAbstractMedia::Source &source,
		    bool isLast);

  std::vector<std::unique_ptr<Source> > sources_;
  std::size_t sourcesRendered_;
  std::string mediaId_;
  WFlags<PlayerOption> flags_;
  MediaPreloadMode preloadMode_;
  std::unique_ptr<WWidget> alternative_;
  bool flagsChanged_, preloadChanged_, sourcesChanged_;

  // Vars received from client
  bool playing_;
  double volume_;
  double current_;
  double duration_;
  bool ended_;
  MediaReadyState readyState_;
  
  static const char *PLAYBACKSTARTED_SIGNAL;
  static const char *PLAYBACKPAUSED_SIGNAL;
  static const char *ENDED_SIGNAL;
  static const char *TIMEUPDATED_SIGNAL;
  static const char *VOLUMECHANGED_SIGNAL;

  void loadJavaScript();
};

W_DECLARE_OPERATORS_FOR_FLAGS(PlayerOption)

}

#endif // WABSTRACT_MEDIA_H_

