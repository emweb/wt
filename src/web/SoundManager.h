// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2009 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef SOUNDMANAGER_H_
#define SOUNDMANAGER_H_

#include <Wt/WMediaPlayer.h>

namespace Wt {

class WApplication;
class WSound;

class SoundManager : public WMediaPlayer
{
public:
  SoundManager();

  void add(WSound *sound);
  void play(WSound *sound, int loops);
  void stop(WSound *sound);

  bool isFinished(WSound *sound) const;
  virtual void refresh() override;

private:
  WSound *current_;

  void setup(WSound *sound);
};

}

#endif // SOUNDMANAGER_H_

