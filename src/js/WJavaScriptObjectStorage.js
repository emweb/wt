/*
 * Copyright (C) 2016 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER
(20, JavaScriptConstructor, "WJavaScriptObjectStorage",
  function(APP, widget) {
    widget.wtJSObj = this;

    var self = this;
    var WT = APP.WT;

    // Deep copy a simple JavaScript object, value or array
    function deepCopy(v) {
      if (jQuery.isArray(v)) {
        var res = [];
	var i;
	for (i = 0; i < v.length; ++i) {
	  res.push(deepCopy(v[i]));
	}
	return res;
      } else if (jQuery.isPlainObject(v)) {
        var res = {};
	var key;
	for (key in v) {
	  if (v.hasOwnProperty(key)) {
	    res[key] = deepCopy(v[key]);
	  }
	}
	return res;
      } else {
        return v;
      }
    }

    // Recursively check if two simple JavaScript objects, arrays or values are equal
    function isEqual(a,b) {
      if (a === b)
        return true;
      if (jQuery.isArray(a) && jQuery.isArray(b)) {
        if (a.length !== b.length)
	  return false;
        var i;
	for (i = 0; i < a.length; ++i) {
	  if (!isEqual(a[i], b[i]))
	    return false;
	}
	return true;
      } else if (jQuery.isPlainObject(a) && jQuery.isPlainObject(b)) {
	 var key;
	 for (key in a) {
	   if (a.hasOwnProperty(key)) {
	     if (!b.hasOwnProperty(key))
	       return false;
	     if (!isEqual(a[key], b[key]))
	       return false;
	   }
	 }
	 for (key in b) {
	   if (b.hasOwnProperty(key)) {
	     if (!a.hasOwnProperty(key))
	       return false;
	   }
	 }
	 return true;
      } else {
        return false;
      }
    }

    // FIXME: hacky check to omit WPainterPaths (WPainterPaths are readonly)
    function isPainterPath(v) {
      return jQuery.isArray(v) && v.length > 6;
    }

    var oldValues = {};
    this.jsValues = [];
    this.setJsValue = function(index, value) {
      if (!isPainterPath(value))
	oldValues[index] = deepCopy(value);
      self.jsValues[index] = value;
    };

    function encodeJSValues() {
      var res = {};
      var value;
      var i;
      for (i = 0; i < self.jsValues.length; ++i) {
        value = self.jsValues[i];
	if (!isPainterPath(value) &&
	    !isEqual(value, oldValues[i])) {
	  res[i] = value;
	}
      }
      return JSON.stringify(res);
    }
    widget.wtEncodeValue = encodeJSValues;
  });
