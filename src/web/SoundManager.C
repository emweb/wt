/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "SoundManager.h"

#include "Wt/WSound"
#include "Wt/WCssDecorationStyle"
#include "Wt/WStringStream"

namespace Wt {

SoundManager::SoundManager(WContainerWidget *parent)
  : WMediaPlayer(WMediaPlayer::Audio, parent)
{
  resize(0, 0);
  setAttributeValue("style", "overflow: hidden");

  controlsWidget()->hide();
  decorationStyle().setBorder(WBorder());

  WStringStream ss;
  ss <<
    "function() { "
    """var s = " << jsRef() << ", l = s.getAttribute('loops');"
    """if (l && l != '0') {"
    ""   "s.setAttribute('loops', l - 1);"
    ""   << jsPlayerRef() << ".jPlayer('play');"
    """}"
    "}";

  ended().connect(ss.str());
  ended().setNotExposed();
}

void SoundManager::add(WSound *sound)
{
  if (getSource(MP3) != sound->url()) {
    clearSources();
    addSource(MP3, WLink(sound->url()));
  }
}

void SoundManager::remove(WSound *sound)
{ }

void SoundManager::play(WSound *sound, int loops)
{
  if (getSource(MP3) != sound->url()) {
    clearSources();
    addSource(MP3, WLink(sound->url()));
  }

  setAttributeValue("loops", std::string());
  setAttributeValue("loops", boost::lexical_cast<std::string>(loops - 1));

  WMediaPlayer::play();
}

void SoundManager::stop(WSound *sound)
{
  WMediaPlayer::stop();
}

bool SoundManager::isFinished(WSound *sound) const
{
  if (getSource(MP3) == sound->url())
    return !playing();
  else
    return true;
}

}
