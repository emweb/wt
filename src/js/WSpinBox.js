/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER
(1, JavaScriptConstructor, "WSpinBox",
 function(APP, edit, precision, prefix, suffix, minValue, maxValue,
      stepValue, decimalPoint, groupSeparator) {
   /** @const */ var TYPE_INTEGER = 0;
   /** @const */ var TYPE_FLOAT = 1;

   /** @const */ var NaNError = "Must be a number";
   /** @const */ var tooSmallError = "The number must be at least ";
   /** @const */ var tooLargeError = "The number may be at most ";

   /** @const */ var CLASS_DOWN = 'dn';
   /** @const */ var CLASS_UP = 'up';
   /** @const */ var CLASS_UNSELECTABLE = 'unselectable';

   edit.wtObj = this;

   var self = this, WT = APP.WT, key_up = 38, key_down = 40, CH = 'crosshair',
     $edit = $(edit);


   var dragStartXY = null, dragStartValue, changed = false;
   var validator = null;
   var isDoubleSpinBox = false;
   var wrapAround = false;

   function isReadOnly() {
     return edit.readOnly;
   }

   function addGrouping(input) {
     var result = "";

     for (var i = 0; i < input.length(); i++) {
       result += input.charAt(i);
     }
   }

   function getValue() {
     var lineEdit = edit.wtLObj;
     var v = "";
     if (lineEdit !== undefined) {
       v = lineEdit.getValue();
       if (v === "") {
     v = prefix + "0" + suffix;
       }
     } else {
       v = edit.value;
     }
     if (v.substr(0, prefix.length) == prefix) {
       v = v.substr(prefix.length);
       if (v.length > suffix.length
       && v.substr(v.length - suffix.length, suffix.length) == suffix) {
     v = v.substr(0, v.length - suffix.length);
     if (groupSeparator) {
       v = v.split(groupSeparator).join("");
     }
     v = v.replace(decimalPoint, ".");
     return Number(v);
       }
     }

     return null;
   }

   function setValue(v) {
     var lineEdit = edit.wtLObj;
     if (v > maxValue)
       if (wrapAround) {
     range = maxValue - minValue ;
     v = minValue + ( (v-minValue) % (range+1) );
       } else {
     v = maxValue;
       }
     else if (v < minValue)
       if (wrapAround) {
     range = maxValue - minValue;
     v = maxValue - ( (Math.abs(v-minValue)-1) % (range+1) )
       } else {
     v = minValue;
       }


     var newValue = v.toFixed(precision);
     newValue = newValue.replace(".", decimalPoint);
     var dotPos = newValue.indexOf(decimalPoint);
     var result = "";
     if (dotPos !== -1) {
       for (var i = 0; i < dotPos; i++) {
     result += newValue.charAt(i);
     if (i < dotPos - 1 && ((dotPos - i - 1) % 3 === 0)) {
       result += groupSeparator;
     }
       }
       result += newValue.substr(dotPos);
     } else {
       result = newValue;
     }

     var oldv = edit.value;
     if (lineEdit !== undefined) {
       lineEdit.setValue(prefix + result + suffix);
     } else {
       edit.value = prefix + result + suffix;
     }

     changed = true;
     self.jsValueChanged(oldv, v);
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

   this.setIsDoubleSpinBox = function(isDouble) {
     isDoubleSpinBox = isDouble;

     this.configure(precision, prefix, suffix, minValue, maxValue, stepValue);
   };

   this.setWrapAroundEnabled = function(enabled) {
     wrapAround = enabled;
   };

   this.configure = function(newPrecision, newPrefix, newSuffix,
                 newMinValue, newMaxValue, newStepValue) {
     precision = newPrecision;
     prefix = newPrefix;
     suffix = newSuffix;
     minValue = newMinValue;
     maxValue = newMaxValue;
     stepValue = newStepValue;

     var Validator = (isDoubleSpinBox ||
              typeof WT.WIntValidator === 'undefined') ?
       WT.WDoubleValidator : WT.WIntValidator;

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
       $edit.addClass(CLASS_UNSELECTABLE);

       dragStartXY = WT.pageCoordinates(event);
       dragStartValue = getValue();
     } else {
       var xy = WT.widgetCoordinates(edit, event);
       if (xy.x > edit.offsetWidth - 16) {
     // suppress selection, focus
     WT.cancelEvent(event);
     WT.capture(edit);
     $edit.addClass(CLASS_UNSELECTABLE);

     var mid = edit.offsetHeight/2;
     if (xy.y < mid)
       WT.eventRepeat(function() { inc(); });
     else
       WT.eventRepeat(function() { dec(); });
       }
     }
   };

   this.mouseUp = function(o, event) {
     $edit.removeClass(CLASS_UNSELECTABLE);
     if (isReadOnly())
       return;

     if (changed || dragStartXY != null) {
       dragStartXY = null;
       changed = false;
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

   this.setLocale = function(point, separator) {
     decimalPoint = point;
     groupSeparator = separator;
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

   this.jsValueChanged = function() {};

   this.setIsDoubleSpinBox(isDoubleSpinBox);
 });
