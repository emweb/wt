/*
 * Copyright (C) 2025 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER(
  1,
  JavaScriptConstructor,
  "WStackedValidator",
  function(validators) {
    this.validate = function(text) {
      for (let i = 0; i < validators.length; ++i) {
        const validator = validators[i];
        const result = validator.validate(text);
        if (!result.valid) {
          return result;
        }
      }

      return { valid: true };
    };
  }
);
