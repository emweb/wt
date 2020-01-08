// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WAUDIO_H_
#define WAUDIO_H_

#include <Wt/WAbstractMedia.h>

namespace Wt {

/*! \class WAudio Wt/WAudio.h Wt/WAudio.h
 *  \brief A widget that plays audio.
 *
 * This is a low-level widget, mapping directly onto a
 * <tt>&lt;audio&gt;</tt> element available in HTML5 compliant
 * browsers.
 *
 * In almost every situation you should use the WMediaPlayer widget
 * if you want the user to be able to interact with the audio, or
 * WSound for simple sound feed-back.
 *
 * Usage of the audio element consists of adding one or more audio
 * sources and setting some options. Since not every browser supports
 * HTML5 audio, the class provides a mechanism to display alternative
 * content in browsers that cannot play the video.
 *
 * \if cpp
 * Usage example:
 * \code
 * WAudio *a = parent->addChild(std::make_unique<WAudio>());
 * a->setOptions(WAudio::Controls);
 * // Addsources may be called multiple times for different formats:
 * // Firefox only plays ogg
 * a->addSource("the_wt_song.ogg");
 * // many others play mp3
 * a->addSource("the_wt_song.mp3", "audio/mp3");
 * // You may display a simple text to explain that you need html5 support...
 * // a->setAlternativeContent(new WText("You have no HTML5 Audio!"));
 * // ... or provide an alternative player, e.g. Flash-based
 * auto f = std::make_unique<WFlashObject>("player.swf");
 * f->setFlashVariable("src", "the_wt_song.mp3");
 * v->setAlternativeContent(std::move(f));
 * \endcode
 * \endif
 *
 * There are two reasons why the a browser may use the alternative
 * content: either because the browser does not support the HTML5
 * audio tag (alternative content is displayed even when JavaScript
 * is not available), or because none of the specified sources contain
 * an audio format that is understood by the browser (requires
 * JavaScript to display the alternative content).
 *
 * The addSource() and setAlternativeContent() may not be called after
 * the widget is rendered.
 *
 * \sa WMediaPlayer
 */
class WT_API WAudio : public WAbstractMedia
{
public:
  /*! \brief Creates a  audio widget.
   *
   * A freshly constructed Audio widget has no media sources,
   * no options, and has preload mode set to PreloadAuto.
   */
  WAudio();

  /*! \brief Returns the JavaScript reference to the audio object, or null.
   *
   * It is possible, for browser compatibility reasons, that jsRef() is
   * not the HTML5 audio element. jsAudioRef() is guaranteed to be an
   * expression that evaluates to the media object. This expression may
   * yield null, if the video object is not rendered at all (e.g. on
   * older versions of Internet Explorer).
   */
  std::string jsAudioRef() const;

protected:
  virtual DomElement *createMediaDomElement() override;
  DomElementType domElementType() const override;

};

}

#endif // WAUDIO_H_

