/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER
(1, JavaScriptConstructor, "WPopupWidget",
 function(APP, el, tr, ahd, shown) {
   jQuery.data(el, 'popup', this);

   var self = this,
       WT = APP.WT,
       hideTimeout = null,
       isTransient = tr,
       autoHideDelay = ahd,
       showF = null, hideF = null;

   function mouseLeave() {
     clearTimeout(hideTimeout);
     if (autoHideDelay > 0)
       hideTimeout = setTimeout(function() { self.hide(); }, autoHideDelay);
   }

   function mouseEnter() {
     clearTimeout(hideTimeout);
   }

   function isHidden() {
     return el.style.display == 'hidden';
   }

   function onDocumentClick(event) {
     function isAncestor(a, b) {
       for (b = b.parentNode; b; b = b.parentNode)
	 if (a == b)
	   return true;
      
       return false;
     }

     if (WT.target(event) == document)
	 return;

     if (!isAncestor(el, WT.target(event)))
       self.hide();
   }

   this.bindShow = function(f) {
     showF = f;
   };

   this.bindHide = function(f) {
     hideF = f;
   };

   this.shown = function(f) {
     if (isTransient) {
       setTimeout(function() {
		    $(document).bind('click', onDocumentClick);
		  }, 0);
     }

     if (showF) showF();
   };

   this.show = function(anchorWidget, side) {
     if (el.style.display != '') {
       el.style.display = '';

       if (anchorWidget)
	 WT.positionAtWidget(el.id, anchorWidget.id, side);

       APP.emit(el, "shown");
     }
   };

   this.hidden = function() {
     if (hideF) hideF();

     if (isTransient)
       $(document).unbind('click', onDocumentClick);     
   };

   this.hide = function() {
     if (el.style.display != 'none') {
       el.style.display = 'none';
     }

     APP.emit(el, "hidden");
     self.hidden();
   };

   this.setTransient = function(t, delay) {
     isTransient = t;
     autoHideDelay = delay;

     if (isTransient && !isHidden())
       setTimeout(function() {
		    $(document).bind('click', onDocumentClick);
		  }, 0);
   };

   $(el).mouseleave(mouseLeave).mouseenter(mouseEnter);

   if (shown)
     this.shown();
 });
