/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */
WT_DECLARE_WT_MEMBER
(1, JavaScriptConstructor, "WAbstractMedia",
 function(APP, el) {
   el.wtObj = this;

   var self = this;
   var WT = APP.WT;

   function handleMove(event) {
   };

   this.play = function() {
     if (el.mediaId) {
       var mediaEl = $('#' + el.mediaId).get(0);
       if (mediaEl) {
         mediaEl.play();
         return;
       }
     }
     if (el.alternativeId) {
       var alternativeEl = $('#' + el.alternativeId).get(0);
       if (alternativeEl && alternativeEl.WtPlay) {
         alternativeEl.WtPlay();
       }
     }
   };

   this.pause = function() {
     if (el.mediaId) {
       var mediaEl = $('#' + el.mediaId).get(0);
       if (mediaEl) {
         mediaEl.pause();
         return;
       }
     }
     if (el.alternativeId) {
       var alternativeEl = $('#' + el.alternativeId).get(0);
       if (alternativeEl && alternativeEl.WtPlay) {
         alternativeEl.WtPause();
       }
     }
    };

   function encodeValue() {
     if (el.mediaId) {
       var mediaEl = $('#' + el.mediaId).get(0);

       if (mediaEl) {
         return '' + mediaEl.volume + ';'
	   + mediaEl.currentTime + ';'
	   + (mediaEl.readyState >= 1 ? mediaEl.duration : 0) + ';'
	   + (mediaEl.paused ? '1' : '0') + ';'
	   + (mediaEl.ended ? ' 1' : '0') + ';'
	   + mediaEl.readyState;
       }
     }

     return null;
   }

   el.wtEncodeValue = encodeValue;
 });
