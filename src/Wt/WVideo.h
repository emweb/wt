// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WVIDEO_H_
#define WVIDEO_H_

#include <Wt/WAbstractMedia.h>

namespace Wt {

/*! \class WVideo Wt/WVideo.h Wt/WVideo.h
 *  \brief A video-playing widget.
 *
 * This is a low-level widget, mapping directly onto a
 * <tt>&lt;video&gt;</tt> element available in HTML5 compliant
 * browsers.
 *
 * In almost every situation you should use the WMediaPlayer widget
 * instead, which has fallback and flexible user-interface options.
 *
 * Usage of the video element consists of adding one or more video
 * sources and setting some options. Since not every browser supports
 * HTML5 video, the class provides a mechanism to display alternative
 * content in browsers that cannot play the video.
 *
 * \if cpp
 * Usage example:
 * \code
 * WVideo *v = addWidget(std::make_unique<WVideo>());
 * v->setOptions(WVideo::Autoplay | WVideo::Controls);
 * // Addsources may be called multiple times for different formats:
 * // Firefox only plays ogg
 * v->addSource("wt.ogv");
 * // many others play mp4
 * v->addSource("wt.mp4", "video/mp4");
 * // Image to be displayed before playback starts
 * v->setPoster("wt.jpg");
 * // You may display a simple text to explain that you need html5 support...
 * // v->setAlternativeContent(new WText("You have no HTML5 Video!"));
 * // ... or provide an alternative player, e.g. Flash-based
 * auto f = std::make_unique<WFlashObject>("player.swf");
 * f->setFlashVariable("startimage", "wt.jpg");
 * f->setFlashVariable("flv", "wt.mp4");
 * f->resize(640, 384);
 * v->setAlternativeContent(std::move(f));
 * \endcode
 * \endif
 *
 * There are two reasons why the a browser may use the alternative
 * content: either because the browser does not support the HTML5
 * video tag (alternative content is displayed even when JavaScript
 * is not available), or because none of the specified sources contain
 * a video format that is understood by the browser (requires
 * JavaScript to display the alternative content).
 *
 * The addSource() and setAlternativeContent() may not be called after
 * the widget is rendered.
 *
 * \sa WMediaPlayer
 */
class WT_API WVideo : public WAbstractMedia
{
public:
  /*! \brief Creates a video widget.
   *
   * The constructor sets the 'controls' option, which causes the browser
   * to display a bar with play/pauze/volume/... controls.
   *
   * A freshly constructed video widget has no poster image, no media
   * sources, has preload mode set to PreloadAuto, and only the
   * Controls flag is set.
   */
  WVideo();
  
  /*! \brief Set the poster image
   *
   * On browsers that support it, the poster image is displayed before
   * the video is playing. Some browsers display the first frame of the
   * video stream once the video stream is loaded; it is therefore a
   * good idea to include the poster image as first frame in the video
   * feed too.
   */
  void setPoster(const std::string &url);
  
  /*! \brief Returns the JavaScript reference to the video object, or null.
   *
   * It is possible, for compatibility reasons, that jsRef() is not
   * the video element. jsVideoRef() is guaranteed to be an expression
   * that evaluates to the video object. This expression may yield
   * null, if the video object is not rendered at all (e.g. on older
   * versions of Internet Explorer).
   */
  std::string jsVideoRef() const;

  virtual void resize(const WLength &width, const WLength &height) override;

protected:
  virtual DomElement *createMediaDomElement() override;
  virtual DomElementType domElementType() const override;
  virtual void updateMediaDom(DomElement& element, bool all) override;

private:
  std::string posterUrl_;
  bool sizeChanged_, posterChanged_;
};

}

#endif // WVIDEO_H_

