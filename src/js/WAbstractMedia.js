/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */
WT_DECLARE_WT_MEMBER(1, JavaScriptConstructor, "WAbstractMedia", function(APP, el) {
  el.wtObj = this;

  const WT = APP.WT;

  this.play = function() {
    if (el.mediaId) {
      const mediaEl = WT.$(el.mediaId);
      if (mediaEl) {
        mediaEl.play();
        return;
      }
    }
    if (el.alternativeId) {
      const alternativeEl = WT.$(el.alternativeId);
      if (alternativeEl && alternativeEl.WtPlay) {
        alternativeEl.WtPlay();
      }
    }
  };

  this.pause = function() {
    if (el.mediaId) {
      const mediaEl = WT.$(el.mediaId);
      if (mediaEl) {
        mediaEl.pause();
        return;
      }
    }
    if (el.alternativeId) {
      const alternativeEl = WT.$(el.alternativeId);
      if (alternativeEl && alternativeEl.WtPlay) {
        alternativeEl.WtPause();
      }
    }
  };

  function encodeValue() {
    if (el.mediaId) {
      const mediaEl = WT.$(el.mediaId);

      if (mediaEl) {
        return "" + mediaEl.volume + ";" +
          mediaEl.currentTime + ";" +
          (mediaEl.readyState >= 1 ? mediaEl.duration : 0) + ";" +
          (mediaEl.paused ? "1" : "0") + ";" +
          (mediaEl.ended ? " 1" : "0") + ";" +
          mediaEl.readyState;
      }
    }

    return null;
  }

  el.wtEncodeValue = encodeValue;
});
