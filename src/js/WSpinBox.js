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

   var self = this, WT = APP.WT, key_up = 38, key_down = 40, CH = 'crosshair',
     $edit = $(edit);

   var dragStartXY = null, dragStartValue;

   function setValue(v) {
     if (v > maxValue)
       v = maxValue;
     else if (v < minValue)
     v = minValue;
     edit.value = v;
   }

   function inc() {
     var v = Number(edit.value);
     v += stepValue;
     setValue(v);
     edit.onchange();
   }

   function dec() {
     var v = Number(edit.value);
     v -= stepValue;
     setValue(v);
     edit.onchange();
   }

   this.update = function(aMin, aMax, aStep) {
     minValue = aMin;
     maxValue = aMax;
     stepValue = aStep;
   };

   this.mouseOut = function(o, event) {
     $edit.removeClass('Wt-spinbox-dn').removeClass('Wt-spinbox-up');
   }

   this.mouseMove = function(o, event) {
     if (!dragStartXY) {
       var xy = WT.widgetCoordinates(edit, event);
       $edit.removeClass('Wt-spinbox-dn').removeClass('Wt-spinbox-up');
       if (xy.x > edit.offsetWidth - 16) {
	 var mid = edit.offsetHeight/2;
	 if (xy.y >= mid - 1 && xy.y <= mid + 1)
	   edit.style.cursor = CH;
	 else {
	   edit.style.cursor = 'default';
	   if (xy.y < mid - 1)
	     $edit.addClass('Wt-spinbox-up');
	   else
	     $edit.addClass('Wt-spinbox-dn');
	 }
       } else
	 edit.style.cursor = '';
     } else {
       var dy = WT.pageCoordinates(event).y - dragStartXY.y;

       var v = dragStartValue;
       v = v - dy*stepValue;

       setValue(v);
     }
   };

   this.mouseDown = function(o, event) {
     if (edit.style.cursor == CH) {
       WT.capture(null);
       WT.capture(edit);

       dragStartXY = WT.pageCoordinates(event);
       dragStartValue = Number(edit.value);
     } else {
       var xy = WT.widgetCoordinates(edit, event);
       if (xy.x > edit.offsetWidth - 16) {
	 // suppress selection, focus
	 WT.cancelEvent(event);
	 WT.capture(edit);
       }
     }
   };

   this.keyDown = function(o, event) {
     if (event.keyCode == key_down)
       dec();
     else if (event.keyCode == key_up)
       inc();
   };

   this.mouseUp = function(o, event) {
     if (dragStartXY != null) {
       dragStartXY = null;
       o.onchange();
     } else {
       var xy = WT.widgetCoordinates(edit, event);
       if (xy.x > edit.offsetWidth - 16) {
	 var mid = edit.offsetHeight/2;
	 if (xy.y < mid)
	   inc();
	 else
	   dec();
       }
     }
   };
 });
