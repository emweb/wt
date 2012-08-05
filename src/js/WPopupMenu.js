/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER
(1, JavaScriptConstructor, "WPopupMenu",
 function(APP, el, autoHideDelay, subMenus) {
   jQuery.data(el, 'obj', this);

   var self = this,
       WT = APP.WT,
       hideTimeout = null,
       location = null,
       entered = 0;

   function doHide() {
     APP.emit(el.id, 'cancel');
   }

   function mouseLeave() {
     --entered;
     if (entered == 0) {
       clearTimeout(hideTimeout);
       hideTimeout = setTimeout(doHide, autoHideDelay);
     }
   }

   function mouseEnter() {
     ++entered;
     clearTimeout(hideTimeout);
   }

   function bindOverEvents(popup)
   {
     $(popup).mouseleave(mouseLeave).mouseenter(mouseEnter);
   }

   this.setHidden = function(hidden) {
     if (hideTimeout) {
       clearTimeout(hideTimeout);
       hideTimeout = null;
     }

     entered = 0;

     if (autoHideDelay > 0 && !hidden) {
       entered = 1;
       if (!location)   // assume we are currently over the location
	 mouseLeave();
     }
   };

   this.popupAt = function(widget) {
     if (autoHideDelay >= 0) {
       if (location != widget) {
	 location = widget;
	 bindOverEvents(location);
	 mouseEnter(); // assume we are currently over the location
       }
     }
   };

   if (autoHideDelay >= 0) {
     setTimeout(function() {
		  bindOverEvents(el);
		  for (var i = 0, il = subMenus.length; i < il; ++i)
		    bindOverEvents(WT.$(subMenus[i]));
		}, 0);
   }
 });
