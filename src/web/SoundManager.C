/*
 * Copyright (C) 2009 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "SoundManager.h"

#include "Wt/WSound.h"
#include "Wt/WCssDecorationStyle.h"
#include "Wt/WStringStream.h"

namespace Wt {

SoundManager::SoundManager()
  : WMediaPlayer(MediaType::Audio)
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

void SoundManager::setup(WSound *sound)
{
  for (unsigned i = 0; i < sound->media_.size(); ++i) {
    const WSound::Source& m = sound->media_[i];
    if (getSource(m.encoding) != m.link) {      
      clearSources();
      for (unsigned j = 0; j < sound->media_.size(); ++j) {
	const WSound::Source& m2 = sound->media_[j];
	addSource(m2.encoding, m2.link);
      }
      break;
    }
  }
}

void SoundManager::add(WSound *sound)
{
  setup(sound);
}

void SoundManager::play(WSound *sound, int loops)
{
  setup(sound);

  setAttributeValue("loops", std::string());
  setAttributeValue("loops", std::to_string(loops - 1));

  current_ = sound;

  WMediaPlayer::play();
}

void SoundManager::stop(WSound *sound)
{
  WMediaPlayer::stop();

  current_ = nullptr;
}

bool SoundManager::isFinished(WSound *sound) const
{
  if (current_ == sound)
    return !playing();
  else
    return true;
}

// There are no visible elements, so ignore refresh
void SoundManager::refresh()
{ }

}
