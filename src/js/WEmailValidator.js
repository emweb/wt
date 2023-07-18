/*
 * Copyright (C) 2023 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

/**
 * @typedef {Object} ValidationResult
 * @property {boolean} valid
 * @property {string|undefined} message
 */

/**
 * @callback validate
 * @param {string} text
 * @returns {ValidationResult}
 */

/**
 * @typedef {Object} Validator
 * @property {validate} validate
 */

WT_DECLARE_WT_MEMBER(
  1,
  JavaScriptConstructor,
  "WEmailValidator",
  /**
   * @param {boolean} mandatory
   * @param {boolean} multiple
   * @param {?string} pattern
   * @param {string} blankError
   * @param {string} invalidError
   * @param {string} notMatchingError
   * @returns {Validator}
   */
  function(mandatory, multiple, pattern, blankError, invalidError, notMatchingError) {
    const emailRegex = (() => {
      const atext = "[a-zA-Z0-9.!#$%&'*+/=?^_`{|}~-]+";
      const let_dig = "[a-zA-Z0-9]";
      const ldh_str = "[a-zA-Z0-9-]{0,61}";
      const label = let_dig + "(?:" + ldh_str + let_dig + ")?";
      const email = "^" + atext + "@" + label + "(?:\\." + label + ")*" + "$";
      return new RegExp(email, "u");
    })();
    const patternRegex = pattern ? new RegExp("^(?:" + pattern + ")$", "u") : null;

    /**
     * Validate a single email address
     *
     * @param {string} emailAddress
     * @returns {boolean}
     */
    function validateOne(emailAddress) {
      return emailRegex.test(emailAddress) &&
        (!patternRegex || patternRegex.test(emailAddress));
    }

    /**
     * @param text
     * @returns {ValidationResult}
     */
    this.validate = function(text) {
      if (text.length === 0) {
        if (mandatory) {
          return { valid: false, message: blankError };
        } else {
          return { valid: true };
        }
      }

      let result;
      if (multiple) {
        result = text.split(",").every(validateOne);
      } else {
        result = validateOne(text);
      }
      if (result) {
        return { valid: true };
      } else if (patternRegex) {
        return { valid: false, message: notMatchingError };
      } else {
        return { valid: false, message: invalidError };
      }
    };
  }
);
