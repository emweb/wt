/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER
(1, "WSpinBox",
 function(APP, edit, minValue, maxValue, stepValue) {
   jQuery.data(edit, 'obj', this);

   var self = this;
   var WT = APP.WT;

   var CH = 'crosshair';

   var dragStartXY = null, dragStartValue;

   this.mouseMove = function(o, event) {
     if (!dragStartXY) {
       var xy = WT.widgetCoordinates(edit, event);
       if (xy.x > edit.offsetWidth - 16) {
	 var mid = edit.offsetHeight/2;
	 if (xy.y >= mid - 1 && xy.y <= mid + 1)
	   edit.style.cursor = CH;
	 else
	   edit.style.cursor = 'default';
       } else
	 edit.style.cursor = '';
     } else {
       var dy = WT.pageCoordinates(event).y - dragStartXY.y;

       var v = dragStartValue;
       v = v - dy*stepValue;

       if (v > maxValue)
	 v = maxValue;
       else if (v < minValue)
	 v = minValue;

       edit.value = v;
     }
   };

   this.mouseDown = function(o, event) {
     if (edit.style.cursor == CH) {
       WT.capture(null);
       WT.capture(edit);

       dragStartXY = WT.pageCoordinates(event);
       dragStartValue = Number(edit.value);
     }
   };

   this.mouseUp = function(o, event) {
     if (dragStartXY != null)
       dragStartXY = null;
     else {
       var xy = WT.widgetCoordinates(edit, event);
       if (xy.x > edit.offsetWidth - 16) {
	 var v = Number(edit.value);

	 var mid = edit.offsetHeight/2;
	 if (xy.y < mid) {
	   v += stepValue;
	 } else {
	   v -= stepValue;
	 }

	 edit.value = v;
       }
     }

     WT.cancelEvent(event);
   };

   this.mouseDblClick = function(o, event) {
     var xy = WT.widgetCoordinates(edit, event);
     if (xy.x > edit.offsetWidth - 16) {
       WT.cancelEvent(event);
     }
   };

 });
