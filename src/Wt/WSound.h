// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2009 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WSOUND_H_
#define WSOUND_H_

#include <Wt/WMediaPlayer.h>

namespace Wt {

class SoundManager;

/*! \class WSound Wt/WSound.h Wt/WSound.h
 * \brief A value class to play a sound effect.
 *
 * This class provides a way to play an MP3 sound asynchonously (if
 * the browser supports this). It is intended as a simple way to play
 * event sounds (not quite for a media center).
 *
 * This class uses a WMediaPlayer to play the sound (using HTML
 * &lt;audio&gt; or a flash player).
 *
 * \if cpp
 * Usage example:
 * \code
 * WSound *s = parent->addChild(std::make_unique<WSound>("djing.mp3"));
 * s->setLoops(3);
 * s->play();
 * playButton->clicked().connect(s, &WSound::play);
 * stopButton->clicked().connect(s, &WSound::stop);
 * \endcode
 * \endif
 */
class WT_API WSound : public WObject
{
public:
  /*! \brief Constructs a sound object.
   *
   * \sa addSource()
   */
  WSound();

  /*! \brief Constructs a sound object for an MP3 media source.
   *
   * The \p url will be assumed to be an MP3 file.
   *
   * \sa addSource()
   */
  WSound(const std::string& url);

  /*! \brief Constructs a sound object.
   *
   * \sa addSource()
   */
  WSound(MediaEncoding encoding, const WLink& link);

  /*! \brief Destructor.
   *
   * Deleting a sound also stops it (if it was playing). 
   */
  ~WSound();

  /*! \brief Adds a media source.
   *
   * You may add multiple media sources (with different encodings) to allow the
   * file to be played in more browsers without needing Flash plugins.
   */
  void addSource(MediaEncoding encoding, const WLink& link);

  /*! \brief Returns the media source.
   *
   * This returns the link set for a specific encoding, or an empty
   * link if no URL was set for that encoding.
   */
  WLink getSource(MediaEncoding encoding) const;

  /*! \brief Sets the amount of times the sound has to be repeated.
   *
   * A call to play() will play the sound \p number of times.
   * The default value is 1 (no repeats).
   */
  void setLoops(int number);

  /*! \brief Returns the configured number of repeats.
   *
   * \a setLoops()
   */
  int loops() const;

  /*! \brief Start asynchronous playback of the sound.
   *
   * This method returns immediately. It will cause the sound to be
   * played for the configured amount of loops().
   *
   * The behavior of play() when a sound is already playing is
   * undefind: it may be intermixed, sequentially queued, or a current
   * playing sound may be stopped. It is recommended to call stop()
   * before play() if you want to avoid mixing multiple instances of a
   * single WSound object.
   */
  void play();

  /*! \brief Stops playback of the sound.
   *
   * This method returns immediately. It causes the current playback (if any)
   * of the sound to be stopped.
   */
  void stop();

private:
  struct Source {
    Source(MediaEncoding anEncoding, WLink aLink)
      : encoding(anEncoding), link(aLink) { }

    MediaEncoding encoding;
    WLink link;
  };

  observing_ptr<SoundManager> manager_;
  std::vector<Source> media_;
  int loops_;

  friend class SoundManager;
};

}

#endif // WSOUND_H_

