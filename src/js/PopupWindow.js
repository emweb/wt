/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER(1, JavaScriptFunction, "PopupWindow", function(WT, url, width, height, onclose) {
  function computePopupPos(width, height) {
    const parentSize = WT.windowSize();

    const xPos = window.screenLeft +
      Math.max(0, Math.floor((parentSize.x - width) / 2));
    const yPos = window.screenTop +
      Math.max(0, Math.floor((parentSize.y - height) / 2));

    return { x: xPos, y: yPos };
  }

  const coordinates = computePopupPos(width, height);
  const w = window.open(
    url,
    "",
    "width=" + width + ",height=" + height +
      ",status=yes,location=yes,resizable=yes,scrollbars=yes" +
      ",left=" + coordinates.x + ",top=" + coordinates.y
  );
  w.opener = window;

  if (onclose) {
    const timer = setInterval(function() {
      if (w.closed) {
        clearInterval(timer);
        onclose(w);
      }
    }, 500);
  }
});
