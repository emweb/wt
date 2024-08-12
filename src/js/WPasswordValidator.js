/*
 * Copyright (C) 2025 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER(
  1,
  JavaScriptConstructor,
  "WPasswordValidator",
  function(mandatory, minLength, maxLength, pattern, blankError, tooSmallError, tooLargeError, patternError) {
    this.validate = function(text) {
      if (text.length === 0) {
        if (mandatory) {
          return { valid: false, message: blankError };
        } else {
          return { valid: true };
        }
      }

      if ((minLength < maxLength || maxLength < 0) && minLength > text.length) {
        return { valid: false, message: tooSmallError };
      }

      if (maxLength > -1 && maxLength < text.length) {
        return { valid: false, message: tooLargeError };
      }

      if (pattern.length > 0) {
        const r = new RegExp(pattern, "v");
        if (!r.test(text)) {
          return { valid: false, message: patternError };
        }
      }

      return { valid: true };
    };
  }
);
