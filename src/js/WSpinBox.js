/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER
(1, JavaScriptConstructor, "WSpinBox",
 function(APP, edit, precision, prefix, suffix, minValue, maxValue,
	  stepValue) {
   /** @const */ var TYPE_INTEGER = 0;
   /** @const */ var TYPE_FLOAT = 1;

   /** @const */ var NaNError = "Must be a number";
   /** @const */ var tooSmallError = "The number must be at least ";
   /** @const */ var tooLargeError = "The number may be at most ";

   /** @const */ var CLASS_DOWN = 'Wt-spinbox-dn';
   /** @const */ var CLASS_UP = 'Wt-spinbox-up';

   jQuery.data(edit, 'obj', this);

   var self = this, WT = APP.WT, key_up = 38, key_down = 40, CH = 'crosshair',
     $edit = $(edit);

   var dragStartXY = null, dragStartValue, changed = false;
   var validator = null;

   function isReadOnly() {
     return !!edit.getAttribute("readonly");
   }

   function getValue() {
     var v = edit.value;
     if (v.substr(0, prefix.length) == prefix) {
       v = v.substr(prefix.length);
       if (v.length > suffix.length
	   && v.substr(v.length - suffix.length, suffix.length) == suffix) {
	 v = v.substr(0, v.length - suffix.length);
	 return Number(v);
       }
     }

     return null;
   }

   function setValue(v) {
     if (v > maxValue)
       v = maxValue;
     else if (v < minValue)
       v = minValue;

     edit.value = prefix + v.toFixed(precision) + suffix;

     changed = true;
   }

   function inc() {
     var v = getValue();
     if (v !== null) {
       v += stepValue;
       setValue(v);
     }
   }

   function dec() {
     var v = getValue();
     if (v !== null) {
       v -= stepValue;
       setValue(v);
     }
   }

   this.update = function(aMin, aMax, aStep, aPrecision) {
     minValue = aMin;
     maxValue = aMax;
     stepValue = aStep;
     precision = aPrecision;

     var Validator = precision == 0 ? WT.WIntValidator : WT.WDoubleValidator;

     validator = new Validator(true, minValue, maxValue,
			       NaNError, NaNError,
			       tooSmallError + minValue,
			       tooLargeError + maxValue);
   };

   this.mouseOut = function(o, event) {
     $edit.removeClass(CLASS_DOWN).removeClass(CLASS_UP);
   };

   this.mouseMove = function(o, event) {
     if (isReadOnly())
       return;

     if (!dragStartXY) {
       var xy = WT.widgetCoordinates(edit, event);

       if ($edit.hasClass(CLASS_DOWN) || $edit.hasClass(CLASS_UP))
	 $edit.removeClass(CLASS_DOWN).removeClass(CLASS_UP);

       if (xy.x > edit.offsetWidth - 16) {
	 var mid = edit.offsetHeight/2;
	 if (xy.y >= mid - 1 && xy.y <= mid + 1)
	   edit.style.cursor = CH;
	 else {
	   edit.style.cursor = 'default';
	   if (xy.y < mid - 1)
	     $edit.addClass(CLASS_UP);
	   else
	     $edit.addClass(CLASS_DOWN);
	 }
       } else
	 if (edit.style.cursor != '')
	   edit.style.cursor = '';
     } else {
       var dy = WT.pageCoordinates(event).y - dragStartXY.y;

       var v = dragStartValue;
       if (v !== null) {
	 v = v - dy*stepValue;
	 setValue(v);
       }
     }
   };

   this.mouseDown = function(o, event) {
     WT.capture(null);
     if (isReadOnly())
       return;

     if (edit.style.cursor == CH) {
       WT.capture(null);
       WT.capture(edit);

       dragStartXY = WT.pageCoordinates(event);
       dragStartValue = getValue();
     } else {
       var xy = WT.widgetCoordinates(edit, event);
       if (xy.x > edit.offsetWidth - 16) {
	 // suppress selection, focus
	 WT.cancelEvent(event);
	 WT.capture(edit);

	 var mid = edit.offsetHeight/2;
	 if (xy.y < mid)
	   WT.eventRepeat(function() { inc(); });
	 else
	   WT.eventRepeat(function() { dec(); });
       }
     }
   };

   this.mouseUp = function(o, event) {
     if (isReadOnly())
       return;

     if (changed || dragStartXY != null) {
       dragStartXY = null;
       o.onchange();
     }

     WT.stopRepeat();
   };

   this.keyDown = function(o, event) {
     if (isReadOnly())
       return;

     if (event.keyCode == key_down)
       WT.eventRepeat(function() { dec(); });
     else if (event.keyCode == key_up)
       WT.eventRepeat(function() { inc(); });
   };

   this.keyUp = function(o, event) {
     if (isReadOnly())
       return;

     if (changed) {
       changed = false;
       o.onchange();
     }

     WT.stopRepeat();
   };

   /*
    * Customized validation function, called from WFormWidget
    */
   this.validate = function(text) {
     var v = getValue();

     if (v === null)
       v = "a"; // NaN

     return validator.validate(v);
   };

   this.update(minValue, maxValue, stepValue, precision);
 });
