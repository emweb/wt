/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER
(1, JavaScriptFunction, "PopupWindow",
 function(WT, url, width, height, onclose) {
   function getScreenPos() {
     var width = 0;
     var height = 0;

     if (typeof (window.screenLeft) === 'number') {
       width = window.screenLeft;
       height = window.screenTop;
     } else if (typeof (window.screenX) === 'number') {
       width = window.screenX;
       height = window.screenY;
     }

     return { x: width, y: height };
   };

   function computePopupPos(width, height) {
     var parentSize = WT.windowSize();
     var parentPos = getScreenPos();

     var xPos = parentPos.x +
       Math.max(0, Math.floor((parentSize.x - width) / 2));
     var yPos = parentPos.y +
       Math.max(0, Math.floor((parentSize.y - height) / 2));

     return { x: xPos, y: yPos };
   }

   var coordinates = computePopupPos(width, height);
   var w = window.open(url, "",
	       "width=" + width + ",height=" + height +
	       ",status=yes,location=yes,resizable=yes,scrollbars=yes" +
	       ",left=" + coordinates.x + ",top=" + coordinates.y);
   w.opener = window;

   if (onclose) {
     var timer = setInterval(function() {
	 if (w.closed) {
	   clearInterval(timer);
	   onclose(w);
	 }
       }, 500);
   }
 });
