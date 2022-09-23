/*
 * Copyright (C) 2016 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER(20, JavaScriptConstructor, "WJavaScriptObjectStorage", function(_APP, widget) {
  widget.wtJSObj = this;

  const self = this;

  // function to determine if the argument is a plain object (an object with zero or more key-value pairs)
  function isPlainObject(object) {
    return Object.prototype.toString.call(object) === "[object Object]";
  }

  // Deep copy a simple JavaScript object, value or array
  function deepCopy(v) {
    if (Array.isArray(v)) {
      const res = [];
      for (let i = 0; i < v.length; ++i) {
        res.push(deepCopy(v[i]));
      }
      return res;
    } else if (isPlainObject(v)) {
      const res = {};
      for (const [key, value] of Object.entries(v)) {
        res[key] = deepCopy(value);
      }
      return res;
    } else {
      return v;
    }
  }

  // Recursively check if two simple JavaScript objects, arrays or values are equal
  function isEqual(a, b) {
    if (a === b) {
      return true;
    }
    if (Array.isArray(a) && Array.isArray(b)) {
      if (a.length !== b.length) {
        return false;
      }
      for (let i = 0; i < a.length; ++i) {
        if (!isEqual(a[i], b[i])) {
          return false;
        }
      }
      return true;
    } else if (isPlainObject(a) && isPlainObject(b)) {
      for (const [key, value] of Object.entries(a)) {
        if (!Object.prototype.hasOwnProperty.call(b, key)) {
          return false;
        }
        if (!isEqual(value, b[key])) {
          return false;
        }
      }
      for (const key of Object.keys(b)) {
        if (!Object.prototype.hasOwnProperty.call(a, key)) {
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
    return Array.isArray(v) && v.length > 6;
  }

  const oldValues = {};
  this.jsValues = [];
  this.setJsValue = function(index, value) {
    if (!isPainterPath(value)) {
      oldValues[index] = deepCopy(value);
    }
    self.jsValues[index] = value;
  };

  function encodeJSValues() {
    const res = {};
    for (let i = 0; i < self.jsValues.length; ++i) {
      const value = self.jsValues[i];
      if (
        !isPainterPath(value) &&
        !isEqual(value, oldValues[i])
      ) {
        res[i] = value;
      }
    }
    return JSON.stringify(res);
  }
  widget.wtEncodeValue = encodeJSValues;
});
