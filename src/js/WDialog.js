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
   var layoutContainer = $(el).find(".dialog-layout").get(0);
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
     }

     if (el.style.position != '')
       el.style.visibility = 'visible';
   };

   function layoutResize(ignored, w, h) {
     if (el.style.position == '') {
       el.style.position = WT.isIE6 ? 'absolute' : 'fixed';
       el.style.visibility = 'visible';
     }

     el.style.height = Math.max(0, h) + 'px';
     el.style.width = Math.max(0, w) + 'px';

     self.centerDialog();
   }

   function wtResize(ignored, w, h) {
     if (w > 0)
       layoutContainer.style.width = w + 'px';
     if (h > 0)
       layoutContainer.style.height = h + 'px';

     self.centerDialog();
   };

   function wtPosition() {
     APP.layouts2.adjust();
   }

   this.onresize = function(w, h) {
     centerX = centerY = false;
     wtResize(el, w, h);

     APP.layouts2.scheduleAdjust();
   };

   layoutContainer.wtResize = layoutResize;
   el.wtPosition = wtPosition;

   if (el.style.width != '')
     layoutContainer.style.width = el.offsetWidth + 'px';

   if (el.style.height != '')
     layoutContainer.style.height = el.offsetHeight + 'px';

   self.centerDialog();
 });
