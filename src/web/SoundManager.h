// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef SOUNDMANAGER_H_
#define SOUNDMANAGER_H_

#include <Wt/WObject>

namespace Wt {

class WApplication;
class WSound;

class SoundManager : public WObject
{
public:
  SoundManager(WApplication *app);
  ~SoundManager();

  void add(WSound *sound);
  void remove(WSound *sound);

  void play(WSound *sound, int loops);
  void stop(WSound *sound);

  bool isFinished(WSound *sound) const;

  int loopsRemaining(WSound *sound) const;

private:
  WApplication *wApp_;
};

}

#endif // SOUNDMANAGER_H_

