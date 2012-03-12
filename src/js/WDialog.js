/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER
(1, JavaScriptConstructor, "WDialog",
 function(APP, el, centerX, centerY) {
   jQuery.data(el, 'obj', this);

   var self = this;
   var titlebar = $(el).find(".titlebar").first().get(0);
   var WT = APP.WT;
   var dsx, dsy;

   function handleMove(event) {
     var e = event||window.event;
     var nowxy = WT.pageCoordinates(e);
     var wxy = WT.windowCoordinates(e);
     var wsize = WT.windowSize();

     if (wxy.x > 0 && wxy.x < wsize.x && wxy.y > 0 && wxy.y < wsize.y) {
       centerX = centerY = false;

       el.style.left = (WT.px(el, 'left') + nowxy.x - dsx) + 'px';
       el.style.top = (WT.px(el, 'top') + nowxy.y - dsy) + 'px';
       el.style.right = '';
       el.style.bottom = '';
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
       return;
     }

     if ((el.style.display != 'none') && (el.style.visibility != 'hidden')) {
       var ws = WT.windowSize();
       var w = el.offsetWidth, h = el.offsetHeight;

       if (centerX) {
	 el.style.left = Math.round((ws.x - w)/2
	     + (WT.isIE6 ? document.documentElement.scrollLeft : 0)) + 'px';
	 el.style.marginLeft = '0px';
       }

       if (centerY) {
	 el.style.top = Math.round((ws.y - h)/2
	     + (WT.isIE6 ? document.documentElement.scrollTop : 0)) + 'px';
	 el.style.marginTop = '0px';
       }

       if (el.style.height != '')
	 wtResize(el, -1, h);

       el.style.visibility = 'visible';
     }
   };

   function wtResize(self, w, h) {
     h -= 2; w -= 2; // 2 = dialog border
     self.style.height = Math.max(0, h) + 'px';
     if (w > 0)
       self.style.width = Math.max(0, w) + 'px';
     var c = $(self).children('.body').get(0);
     var t = $(self).children('.titlebar').get(0);
     if (t)
       h -= t.offsetHeight + 8; // 8 = body padding
     if (h > 0) {
       c.style.height = h + 'px';
       if (APP.layouts)
	 APP.layouts.adjust();
     }
   };

   this.onresize = function(w, h) {
     centerX = centerY = false;
     wtResize(el, w, h);
   };

   el.wtResize = wtResize;
   el.wtPosition = this.centerDialog;
 });
