/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */
WT_DECLARE_WT_MEMBER
(1, JavaScriptConstructor, "WMediaPlayer",
 function(APP, el) {
   el.wtObj = this;

   var self = this;
   var WT = APP.WT;

   function encodeValue() {
     var jplayer = $(el).find(".jp-jplayer").data("jPlayer"),
         status = jplayer.status,
	 options = jplayer.options;

     return options.volume
       + ';' + status.currentTime
       + ';' + status.duration
       + ';' + (status.paused ? 1 : 0)
       + ';' + (status.ended ? 1 : 0)
       + ';' + status.readyState
       + ';' + (status.playbackRate ? status.playbackRate : 1)
       + ';' + status.seekPercent;
   }

   el.wtEncodeValue = encodeValue;

   function setPlaybackRate(rate) {
     var jplayer = this,
         media = jplayer.htmlElement.video || jplayer.htmlElement.audio;

     if (media)
       media.playbackRate = rate;

     return this;
   };

   var jplayer = $(el).find(".jp-jplayer").data("jPlayer");
   jplayer.wtPlaybackRate = setPlaybackRate;

 });
