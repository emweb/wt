/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER
(1, JavaScriptConstructor, "WPopupMenu",
 function(APP, el, autoHideDelay) {
   jQuery.data(el, 'obj', this);

   var self = this,
       WT = APP.WT,
       hideTimeout = null,
       entered = false;

   function doHide() {
     APP.emit(el, 'cancel');
   }

   this.setHidden = function(hidden) {
     if (hideTimeout)
       clearTimeout(hideTimeout);
     hideTimeout = null;
   }

   if (autoHideDelay >= 0) {
     $(document).find('.Wt-popupmenu')
       .mouseleave(function() {
	   if (entered) {
	     clearTimeout(hideTimeout);
	     hideTimeout = setTimeout(doHide, autoHideDelay);
	   }
	 })
       .mouseenter(function() {
	   entered = true;
	   clearTimeout(hideTimeout);
	 });
   }
 });
