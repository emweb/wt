/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "WSound"
#include "WApplication"
#include "SoundManager.h"

namespace Wt {

WSound::WSound(WObject *parent)
  : WObject(parent),
    loops_(1)
{ }

WSound::WSound(const std::string &url, WObject *parent)
  : WObject(parent),
    loops_(1)
{
  addSource(WMediaPlayer::MP3, WLink(url));
}

WSound::WSound(WMediaPlayer::Encoding encoding, const WLink& link, WObject *parent)
  : WObject(parent),
    loops_(1)
{
  addSource(encoding, link);
}

WSound::~WSound()
{
  stop();
}

void WSound::addSource(WMediaPlayer::Encoding encoding, const WLink& link)
{
  media_.push_back(Source(encoding, link));

  wApp->getSoundManager()->add(this);
}

WLink WSound::getSource(WMediaPlayer::Encoding encoding) const
{
  for (unsigned i = 0; i < media_.size(); ++i) {
    if (media_[i].encoding == encoding)
      return media_[i].link;
  }

  return WLink();
}

std::string WSound::url() const
{
  return getSource(WMediaPlayer::MP3).url();
}

//bool isFinished() const;

int WSound::loops() const
{
  return loops_;
}

//int loopsRemaining() const;

void WSound::setLoops(int number)
{
  loops_ = number;
}

void WSound::play()
{  
  wApp->getSoundManager()->play(this, loops_);
}

void WSound::stop()
{
  wApp->getSoundManager()->stop(this);
}

}
