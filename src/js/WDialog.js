/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER
(1, "WDialog",
 function(APP, el) {
   jQuery.data(el, 'obj', this);

   var self = this;
   var titlebar = $(el).find(".titlebar").first().get(0);
   var WT = APP.WT;
   var dsx, dsy;
   var moved = false;

   if (el.style.left != '' || el.style.top != '')
     moved = true;

   function handleMove(event) {
     var e = event||window.event;
     var nowxy = WT.pageCoordinates(e);
     var wxy = WT.windowCoordinates(e);
     var wsize = WT.windowSize();

     if (wxy.x > 0 && wxy.x < wsize.x && wxy.y > 0 && wxy.y < wsize.y) {
       moved = true;

       el.style.left = (WT.pxself(el, 'left') + nowxy.x - dsx) + 'px';
       el.style.top = (WT.pxself(el, 'top') + nowxy.y - dsy) + 'px';
       dsx = nowxy.x;
       dsy = nowxy.y;
     }
   };

   if (titlebar) {
     titlebar.onmousedown = function(event) {
       var e = event||window.event;
       WT.capture(titlebar);
       var pc = WT.pageCoordinates(e);
       dsx = pc.x;
       dsy = pc.y;

       titlebar.onmousemove = handleMove;
     };

     titlebar.onmouseup = function(event) {
       titlebar.onmousemove = null;

       WT.capture(null);
     };
   }

   this.centerDialog = function() {
     if (el.parentNode == null) {
       el = titlebar = null;
       this.centerDialog = function() { };
       return;
     }

     if (el.style.display != 'none') {
       if (!moved) {
	 var ws = WT.windowSize();
	 var w = el.offsetWidth, h = el.offsetHeight;
	 el.style.left = Math.round((ws.x - w)/2
	   + (WT.isIE6 ? document.documentElement.scrollLeft : 0)) + 'px';
	 el.style.top = Math.round((ws.y - h)/2
	   + (WT.isIE6 ? document.documentElement.scrollTop : 0)) + 'px';
	 el.style.marginLeft='0px';
	 el.style.marginTop='0px';

	 if (el.style.width != '' && el.style.height != '')
	   self.wtResize(el, w, h);
       }
       el.style.visibility = 'visible';
     }
   };

   this.wtResize = function(self, w, h) {
     h -= 2; w -= 2; // 2 = dialog border
     self.style.height= h + 'px';
     self.style.width= w + 'px';
     var c = self.lastChild;
     var t = c.previousSibling;
     h -= t.offsetHeight + 8; // 8 = body padding
     if (h > 0) {
       c.style.height = h + 'px';
       if (APP.layouts)
	 APP.layouts.adjust();
     }
   };
 });
