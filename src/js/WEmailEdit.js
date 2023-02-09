/*
 * Copyright (C) 2023 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER(
  1,
  JavaScriptFunction,
  "encodeEmailValue", /**
   * @param {HTMLInputElement} emailInput
   * @returns {string}
   */
  function(emailInput) {
    /**
     * Strip newlines, as per:
     * https://infra.spec.whatwg.org/#strip-newlines
     *
     * @param {string} text
     * @returns {string}
     */
    function stripNewlines(text) {
      return text.replaceAll(new RegExp("[\\r\\n]+", "g"), "");
    }

    /**
     * Strip leading and trailing ASCII whitespace, as per:
     * https://infra.spec.whatwg.org/#strip-leading-and-trailing-ascii-whitespace
     * https://infra.spec.whatwg.org/#ascii-whitespace
     *
     * This doesn't do quite the same thing as a normal trim, which considers more characters as whitespace.
     *
     * @param {string} text
     * @returns {string}
     */
    function asciiTrim(text) {
      return text.replaceAll(new RegExp("(^[\\t\\r\\f\\n ]+)|([\\t\\r\\f\\n ]+$)", "g"), "");
    }

    /**
     * Perform sanitization, as per:
     * https://html.spec.whatwg.org/multipage/input.html#email-state-(type=email)
     *
     * @param {string} text
     * @returns {string}
     */
    function sanitize(text) {
      if (emailInput.multiple) {
        return text.split(",").map(asciiTrim).join(",");
      } else {
        return asciiTrim(stripNewlines(text));
      }
    }

    return sanitize(emailInput.value);
  }
);
