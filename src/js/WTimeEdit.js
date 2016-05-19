/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER
(1, JavaScriptConstructor, "WTimeEdit",
 function(APP, edit, popup) {
   /** @const */ var CLASS_HOVER = 'hover';
   /** @const */ var CLASS_ACTIVE = 'active';
   /** @const */ var CLASS_UNSELECTABLE = 'unselectable';

   jQuery.data(edit, 'dobj', this);

   var self = this, WT = APP.WT, $edit = $(edit);

   function isReadOnly() {
	 return edit.readOnly;
   }

   function isPopupVisible() {
     return $('#' + popup).style.display === '';
   }

   function getPopup() {
     var p = $('#' + popup).get(0);
     return jQuery.data(p, 'popup');
   }

   function resetButton() {
     $edit.removeClass(CLASS_ACTIVE);
   }

   function showPopup() {
     var p = getPopup();
     p.bindHide(resetButton);
     p.show(edit, WT.Vertical);
   }

   this.mouseOut = function(o, event) {
     $edit.removeClass(CLASS_HOVER);
   };

   this.mouseMove = function(o, event) {
     if (isReadOnly())
       return;

     var xy = WT.widgetCoordinates(edit, event);

     if (xy.x > edit.offsetWidth - 40) {
       $edit.addClass(CLASS_HOVER);
     } else {
       if ($edit.hasClass(CLASS_HOVER))
     $edit.removeClass(CLASS_HOVER);
     }
   };

   this.mouseDown = function(o, event) {
     if (isReadOnly())
       return;

     var xy = WT.widgetCoordinates(edit, event);
     if (xy.x > edit.offsetWidth - 40) {
       $edit.addClass(CLASS_UNSELECTABLE).addClass(CLASS_ACTIVE);
     }
   };

   this.mouseUp = function(o, event) {
     $edit.removeClass(CLASS_UNSELECTABLE);

     var xy = WT.widgetCoordinates(edit, event);
     if (xy.x > edit.offsetWidth - 40)
       showPopup();
   };
 });
