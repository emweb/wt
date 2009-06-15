/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "WSound"
#include "WApplication"
#include "SoundManager.h"

namespace Wt {

WSound::WSound(const std::string &url, WObject *parent)
  : WObject(parent),
  url_(url),
  loops_(1)
{
  sm_ = wApp->getSoundManager();
  sm_->add(this);
}

WSound::~WSound()
{
  stop();
  sm_->remove(this);
}

const std::string &WSound::url() const
{
  return url_;
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
  sm_->play(this, loops_);
}

void WSound::stop()
{
  sm_->stop(this);
}

}
