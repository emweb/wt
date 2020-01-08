/*
 * Copyright (C) 2009 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "WSound.h"
#include "WApplication.h"
#include "SoundManager.h"

namespace Wt {

WSound::WSound()
  : manager_(WApplication::instance()->getSoundManager()),
    loops_(1)
{ }

WSound::WSound(const std::string &url)
  : manager_(WApplication::instance()->getSoundManager()),
    loops_(1)
{
  addSource(MediaEncoding::MP3, WLink(url));
}

WSound::WSound(MediaEncoding encoding, const WLink& link)
  : manager_(WApplication::instance()->getSoundManager()),
    loops_(1)
{
  addSource(encoding, link);
}

WSound::~WSound()
{
  stop();
}

void WSound::addSource(MediaEncoding encoding, const WLink& link)
{
  media_.push_back(Source(encoding, link));
  if (manager_)
    manager_->add(this);
}

WLink WSound::getSource(MediaEncoding encoding) const
{
  for (unsigned i = 0; i < media_.size(); ++i) {
    if (media_[i].encoding == encoding)
      return media_[i].link;
  }

  return WLink();
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
  if (manager_)
    manager_->play(this, loops_);
}

void WSound::stop()
{
  if (manager_)
    manager_->stop(this);
}

}
