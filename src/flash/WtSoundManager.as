/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/*
 * To compile:
 * mtasc -main -swf WtSoundManager.swf WtSoundManager.as -version 8 -header 16:16:30
 */
import flash.external.ExternalInterface;

class WtSound {
  static var app: WtSound;

  static function main(mc) {
    app = new WtSound();
  }

  function WtSound() {
    var JSObject = "WtSoundManager";
    var sounds = [];

    var add = function(ID, url)
    {
      sounds[ID] = new Sound();
      sounds[ID].setVolume(100);
      sounds[ID].loadSound(url, false); // loops don't work with streaming
      sounds[ID].doPlay = false;
      sounds[ID].onLoad = function(success) {
        if (success) {
          sounds[ID].isLoaded = true;
          if (sounds[ID].doPlay) {
            sounds[ID].start(0, sounds[ID].loops);
          }
        }
      }
    }

    var remove = function(ID)
    {
      if (sounds[ID]) {
        delete sounds[ID];
      }
    }

    var play = function(ID, loops)
    {
      if (sounds[ID].isLoaded) {
        sounds[ID].start(0, parseInt(loops));
      } else {
        sounds[ID].doPlay = true;
        sounds[ID].loops = loops;
      }
    }

    var stop = function(ID)
    {
      sounds[ID].stop();
    }

    ExternalInterface.addCallback('WtAdd', this, add);
    ExternalInterface.addCallback('WtRemove', this, remove);
    ExternalInterface.addCallback('WtPlay', this, play);
    ExternalInterface.addCallback('WtStop', this, stop);
    ExternalInterface.call(JSObject + ".flashInitializedCB");
  }
}
