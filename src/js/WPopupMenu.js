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
       entered = 0;

   function doHide() {
     console.log(el.id + "doHide");
     APP.emit(el.id, 'cancel');
   }

   function mouseLeave() {
     --entered;
     console.log(el.id + ": mouseleave " + entered);
     if (entered == 0) {
       clearTimeout(hideTimeout);
       hideTimeout = setTimeout(doHide, autoHideDelay);
     }
   }

   function mouseEnter() {
     ++entered;
     console.log(el.id + ": mouseenter " + entered);
     clearTimeout(hideTimeout);
   }
   
   function bindOverEvents(popup)
   {
     $(popup).mouseleave(mouseLeave).mouseenter(mouseEnter);
   }

   this.setHidden = function(hidden) {
     if (hideTimeout)
       clearTimeout(hideTimeout);
     hideTimeout = null;
     entered = 0;
   };
   
   if (autoHideDelay >= 0) {
     bindOverEvents(el);
     for (var i = 0, il = subMenus.length; i < il; ++i)
       bindOverEvents(WT.$(subMenus[i]));
   }
 });
