/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "SoundManager.h"
#include "Wt/WSound"
#include "Wt/WApplication"
#include "Wt/WFlashObject"

namespace Wt {

SoundManager::SoundManager(WApplication *app)
  : WObject(app),
    wApp_(app)
{
  WFlashObject *player_
    = new WFlashObject(WApplication::resourcesUrl() + "WtSoundManager.swf",
		       wApp_->domRoot());
  player_->setAlternativeContent(0);

  player_->resize(100, 100);
  player_->setPositionScheme(Wt::Absolute);
  player_->setOffsets(-900, Wt::Left | Wt::Top);
  player_->setFlashParameter("allowScriptAccess", "always");
  player_->setFlashParameter("quality", "high");
  player_->setFlashParameter("bgcolor", "#aaaaaa");
  player_->setFlashParameter("wmode", "");

  wApp_->doJavaScript(
    """WtSoundManager = {};"
    """WtSoundManager.initialized = false;"
    """WtSoundManager.queue = new Array();"
    """WtSoundManager.player = null;"
    """WtSoundManager.flashInitializedCB = function() {"
    ""  "WtSoundManager.initialized = true;"
    ""  "WtSoundManager.player = " + player_->jsFlashRef() + ";"
    ""  "var i, il;"
    ""  "for (i = 0, il = WtSoundManager.queue.length; i < il; i++) {"
    ""    "var action = WtSoundManager.queue[i].action;"
    ""    "if (action == 'add') {"
    ""      "WtSoundManager.add(WtSoundManager.queue[i].id, "
    ""                       "WtSoundManager.queue[i].url);"
    ""    "} else if (action == 'remove') {"
    ""      "WtSoundManager.remove(WtSoundManager.queue[i].id);"
    ""    "} else if (action == 'play') {"
    ""      "WtSoundManager.doPlay(WtSoundManager.queue[i].id, "
    ""                          "WtSoundManager.queue[i].loops);"
    ""    "} else if (action == 'stop') {"
    ""      "WtSoundManager.doStop(WtSoundManager.queue[i].id);"
    ""    "} else {"
    ""      "alert('WWtSoundManager internal error: action not found: ' "
    ""            "+ action);"
    ""    "}"
    ""  "}"
    """};"
    """WtSoundManager.onerror = function() {"
    ""  "alert('WtSoundManager failed to start');"
    """};"
    """WtSoundManager.add = function(id, url) {"
    ""  "if(WtSoundManager.initialized) {"
    ""    "try {"
    ""      "WtSoundManager.player.WtAdd(id, url);"
    ""    "} catch (e) {}"
    ""  "} else {"
    ""    "WtSoundManager.queue.push({action: 'add', id: id, url: url});"
    ""  "}"
    """};"
    """WtSoundManager.remove = function(id) {"
    ""  "if (WtSoundManager.initialized) {"
    ""    "try {"
    ""      "WtSoundManager.player.WtRemove(id);"
    ""    "} catch (e) {}"
    ""  "} else {"
    ""    "WtSoundManager.queue.push({action: 'remove', id: id});"
    ""  "}"
    """};\n"
    """WtSoundManager.doPlay = function(id, loops) {\n"
    ""  "if (WtSoundManager.initialized) {\n"
    ""    "try {"
    ""      "WtSoundManager.player.WtPlay(id, loops);\n"
    ""    "} catch (e) {}"
    ""  "} else {\n"
    ""    "WtSoundManager.queue.push({action: 'play', id: id, loops: loops});\n"
    ""  "}\n"
    """};\n"
    """WtSoundManager.doStop = function(id) {"
    ""  "if (WtSoundManager.initialized) {"
    ""    "try {"
    ""      "WtSoundManager.player.WtStop(id);"
    ""    "} catch (e) {}"
    ""  "} else {"
    ""    "WtSoundManager.queue.push({action: 'stop', id: id});"
    ""  "}"
    """};",
    false
  );

}

SoundManager::~SoundManager()
{
}

void SoundManager::add(WSound *sound)
{
  std::stringstream ss;
  ss << "WtSoundManager.add(\"" << sound->id() << "\", \""
    << sound->url() << "\");";
  wApp_->doJavaScript(ss.str());
}

void SoundManager::remove(WSound *sound)
{
  std::stringstream ss;
  ss << "WtSoundManager.remove(\"" << sound->id() << "\", \""
    << sound->url() << "\");";
  wApp_->doJavaScript(ss.str());
}

void SoundManager::play(WSound *sound, int loops)
{
  std::stringstream ss;
  ss << "WtSoundManager.doPlay(\"" << sound->id() << "\", "
    << sound->loops() << ");";
  wApp_->doJavaScript(ss.str());
}

void SoundManager::stop(WSound *sound)
{
  std::stringstream ss;
  ss << "WtSoundManager.doStop(\"" << sound->id() << "\");";
  wApp_->doJavaScript(ss.str());
}

bool SoundManager::isFinished(WSound *sound) const
{
  return true; // FIXME!
}

int SoundManager::loopsRemaining(WSound *sound) const
{
  return 0; // FIXME!
}

}
