/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER
(1, JavaScriptConstructor, "WDatePicker",
 function(APP, edit, popupId, global) {
   jQuery.data(edit, 'obj', this);

   var self = this, WT = APP.WT, $edit = $(edit);

   function isReadOnly() {
     return !!edit.getAttribute("readonly");
   }

   function isOverIcon(event) {
     if (isReadOnly())
       return false;

     var xy = WT.widgetCoordinates(edit, event);

     return (xy.x > edit.offsetWidth - 16);
   }

   this.mouseMove = function(o, event) {
     if (isOverIcon(event)) {
       edit.style.cursor = 'default';
     } else {
       if (edit.style.cursor != '')
	 edit.style.cursor = '';
     }
   };

   this.mouseClick = function(o, event) {
     if (isOverIcon(event))
       self.showPopup();
   };

   this.showPopup = function() {
     APP.emit(edit, 'popup');
     WT.positionAtWidget(popupId, edit.id, WT.Vertical, global, true);
   };
 });
