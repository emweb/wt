/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER
(1, JavaScriptConstructor, "WPopupWidget",
 function(APP, el, tr, ahd, shown) {
   el.wtPopup = this;

   var self = this,
       WT = APP.WT,
       hideTimeout = null,
       isTransient = tr,
       autoHideDelay = ahd,
       touch = null,
       showF = null, hideF = null;

   function bindDocumentClick() {
     if (WT.isIOS) {
       $(document).bind('touchstart', startTouch);
       $(document).bind('touchend', endTouch);
     } else
       $(document).bind('click', onDocumentClick);
   }

   function unbindDocumentClick() {
     if (WT.isIOS) {
       $(document).unbind('touchstart', startTouch);
       $(document).unbind('touchend', endTouch);
     } else
       $(document).unbind('click', onDocumentClick);
   }

   function startTouch(event) {
     var l = event.originalEvent.touches;
     if (l.length > 1)
       touch = null;
     else {
       touch = {
         x: l[0].screenX,
         y: l[0].screenY
       }
     }
   }

   function endTouch(event) {
     if (touch) {
       var t = event.originalEvent.changedTouches[0];
       if (Math.abs(touch.x - t.screenX) < 20 && Math.abs(touch.y - t.screenY) < 20)
         onDocumentClick(event);
     }
   }

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
       if (a == b)
	 return true;

       for (b = b.parentNode; b; b = b.parentNode)
	 if (a == b)
	   return true;
      
       return false;
     }

     var target = WT.target(event);
     if (target == document)
       if (WT.WPopupWidget.popupClicked !== null)
	 target = WT.WPopupWidget.popupClicked;

     if (!isAncestor(el, target))
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
 		    bindDocumentClick();
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
       unbindDocumentClick();
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
		    bindDocumentClick();
		  }, 0);
   };

   $(el).mouseleave(mouseLeave).mouseenter(mouseEnter);

   if (shown)
     this.shown();
 });
